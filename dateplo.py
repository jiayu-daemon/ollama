import sys
import time
from datetime import datetime


def main() -> None:
    try:
        while True:
            current_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            sys.stdout.write(f"\r当前时间：{current_time}")
            sys.stdout.flush()
            time.sleep(1)
    except KeyboardInterrupt:
        sys.stdout.write("\n")
        sys.stdout.flush()


if __name__ == "__main__":
    main()
