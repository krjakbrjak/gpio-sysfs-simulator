#define FUSE_USE_VERSION 34

#include "driver.h"
#include <cstring>
#include <fmt/format.h>
#include <fuse3/fuse.h>
#include <iostream>
#include <mutex>
#include <regex>
#include <string>
#include <sys/epoll.h>
#include <sys/types.h>
#include <unistd.h>
#include <unordered_map>

namespace {
const std::regex GPIO_PATTERN{"/gpio(\\d+)$"};
const std::regex VALUE_PATTERN{"/gpio(\\d+)/value$"};
const std::regex DIRECTION_PATTERN{"/gpio(\\d+)/direction$"};

const std::string EXPORT = "export";
const std::string UNEXPORT = "unexport";
const std::string EXPORT_PATH = "/export";
const std::string UNEXPORT_PATH = "/unexport";

std::mutex g_mutex;
/**
 * Maps path to poll handles.
 */
std::unordered_map<std::string, fuse_pollhandle *> poll_handles;
/**
 * Stores true if the value of the pin (under path) was modified.
 */
std::unordered_map<std::string, bool> poll_ready;

void notify(const std::string &path) {
    if (poll_handles.count(path)) {
        auto ph = poll_handles[path];
        poll_ready[path] = true;
        ::fuse_notify_poll(ph);
    }
}

unsigned set_events(const std::string &path, unsigned events) {
    if (poll_ready[path]) {
        poll_ready[path] = false;
        return events;
    }

    return 0;
}

int do_getattr(const char *path, struct stat *st, fuse_file_info *) {
    std::lock_guard<std::mutex> lock{g_mutex};

    st->st_uid = getuid();
    st->st_gid = getgid();
    st->st_atime = time(NULL);
    st->st_mtime = time(NULL);

    std::string filename{path};
    if (std::smatch match; std::regex_search(filename, match, GPIO_PATTERN) || filename == "/") {
        st->st_mode = S_IFDIR | 0755;
        // On Linux the number of links to a directory is always 2 + n, because of '.' and '..'
        st->st_nlink = 2;
    } else {
        st->st_mode = S_IFREG | 0644;
        st->st_nlink = 1;
        st->st_size = 1024;
    }

    return 0;
}

int do_readdir(const char *path, void *buffer, fuse_fill_dir_t filler,
               off_t offset, struct fuse_file_info *fi,
               enum fuse_readdir_flags flags) {
    std::lock_guard<std::mutex> lock{g_mutex};

    filler(buffer, ".", NULL, 0, FUSE_FILL_DIR_PLUS);
    filler(buffer, "..", NULL, 0, FUSE_FILL_DIR_PLUS);

    std::string filename{path};
    auto &driver = ::gpio::Driver::get();
    if (filename == "/") {
        filler(buffer, EXPORT.c_str(), NULL, 0, FUSE_FILL_DIR_PLUS);
        filler(buffer, UNEXPORT.c_str(), NULL, 0, FUSE_FILL_DIR_PLUS);
        for (auto pin : driver.get_pins()) {
            filler(buffer, ::fmt::format("gpio{}", pin).c_str(), NULL, 0, FUSE_FILL_DIR_PLUS);
        }
    } else if (std::smatch match; std::regex_search(filename, match, GPIO_PATTERN)) {
        filler(buffer, "value", NULL, 0, FUSE_FILL_DIR_PLUS);
        filler(buffer, "direction", NULL, 0, FUSE_FILL_DIR_PLUS);
    }

    return 0;
}

int do_read(const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi) {
    std::lock_guard<std::mutex> lock{g_mutex};

    std::string filename{path};
    auto &driver = ::gpio::Driver::get();
    if (filename == EXPORT_PATH || filename == UNEXPORT_PATH) {
        memcpy(buffer, "", 0);
        return 0;
    } else if (std::smatch match; std::regex_search(filename, match, VALUE_PATTERN)) {
        auto pin = std::stoi(match[1]);
        auto value = std::to_string(driver.get_value(pin));
        memcpy(buffer, value.c_str(), value.size());
        return 1;
    } else if (std::smatch match; std::regex_search(filename, match, DIRECTION_PATTERN)) {
        auto pin = std::stoi(match[1]);
        auto value = ::gpio::to_string(driver.get_direction(pin));
        memcpy(buffer, value.c_str(), value.size());
        return value.size();
    }

    return -1;
}

int do_write(const char *path, const char *buffer, size_t size, off_t offset,
             struct fuse_file_info *fi) {
    std::lock_guard<std::mutex> lock{g_mutex};

    std::string filename{path};
    auto &driver = ::gpio::Driver::get();
    if (filename == EXPORT_PATH) {
        try {
            auto pin = std::stoi(std::string{buffer, buffer + size});
            driver.export_pin(pin);
            return size;
        } catch (...) {
            return -1;
        }
    } else if (filename == UNEXPORT_PATH) {
        try {
            auto pin = std::stoi(std::string{buffer, buffer + size});
            driver.unexport_pin(pin);
            return size;
        } catch (...) {
            return -1;
        }
    } else if (std::smatch match; std::regex_search(filename, match, VALUE_PATTERN)) {
        try {
            auto pin = std::stoi(match[1]);
            if (auto value = std::stoi(std::string{buffer, buffer + size}); driver.get_value(pin) != value) {
                driver.set_value(pin, value);
                notify(path);
            }
            return size;
        } catch (...) {
            return -1;
        }
    } else if (std::smatch match; std::regex_search(filename, match, DIRECTION_PATTERN)) {
        try {
            auto pin = std::stoi(match[1]);
            if (auto value = ::gpio::from_string(buffer, size); driver.get_direction(pin) != value) {
                driver.set_direction(pin, value);
                notify(path);
            }
            return size;
        } catch (...) {
            return -1;
        }
    }

    return -1;
}

int do_poll(const char *path, struct fuse_file_info *fi,
            struct fuse_pollhandle *ph, unsigned *reventsp) {
    std::lock_guard<std::mutex> lock{g_mutex};

    if (ph != NULL) {
        if (poll_handles.count(path) != 0) {
            ::fuse_pollhandle_destroy(poll_handles[path]);
        }
        poll_handles[path] = ph;
    }

    *reventsp |= set_events(path, EPOLLET | EPOLLIN);

    return 0;
}
} // namespace

int main(int argc, char *argv[]) {
    fuse_args args = FUSE_ARGS_INIT(argc, argv);

    fuse_operations operations = {
        .getattr = do_getattr,
        .read = do_read,
        .write = do_write,
        .readdir = do_readdir,
        .poll = do_poll,
    };

    return fuse_main(argc, argv, &operations, NULL);
}
