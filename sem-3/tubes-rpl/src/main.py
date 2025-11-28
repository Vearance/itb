import sys

from controllers.MainController import MainController


def main():
    controller = MainController("tabungin.db")
    exit_code = controller.start()
    # Exit with the application exit code
    sys.exit(exit_code)


if __name__ == "__main__":
    main()
