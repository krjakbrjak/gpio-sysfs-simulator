import argparse
import os
import select

def create_file_descriptor(file_path):
    file_descriptor = os.open(file_path, os.O_RDONLY | os.O_NONBLOCK)
    return file_descriptor

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-p", "--pins", nargs="+", help="Pins to watch", required=True)
    parser.add_argument("-m", "--mount", help="Mount point", required=True)

    args = parser.parse_args()

    mapping = {}
    epoll = select.epoll()
    for pin in args.pins:
        with open(os.path.join(args.mount, "export"), 'w') as f:
            f.writelines([pin])
        with open(os.path.join(args.mount, f"gpio{pin}", "direction"), 'w') as f:
            f.writelines(["out"])
        path = os.path.join(args.mount, f"gpio{pin}", "value")
        file_descriptor = create_file_descriptor(path)
        mapping.update({file_descriptor: path})
        epoll.register(file_descriptor, select.EPOLLIN | select.EPOLLET)

    try:
        print(f"Listening for events on pins: [{', '.join(args.pins)}]")
        while True:
            events = epoll.poll()

            for fileno, event in events:
                if event & select.EPOLLIN or event & select.EPOLLET:
                    with open(mapping[fileno]) as f:
                        data = f.read()
                        print(f"Read data from file {mapping[fileno]}: {data}")
    finally:
        for fd, _ in mapping.items():
            epoll.unregister(fd)
        epoll.close()
