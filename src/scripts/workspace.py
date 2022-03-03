import os
import pathlib


def get_docs_dir():
    return os.path.join(pathlib.Path.home(), "Documents")


def get_desktop_dir():
    return os.path.join(pathlib.Path.home(), "Desktop")