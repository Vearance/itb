import sys
import os
from PyQt6.QtWidgets import QApplication


current_dir = os.path.dirname(os.path.abspath(__file__))

project_root = os.path.dirname(current_dir)
os.chdir(project_root)

sys.path.append(current_dir)

from views.TargetView import ProgressView

def main():
    app = QApplication(sys.argv)

    window = ProgressView()
    window.show()
    
    sys.exit(app.exec())

if __name__ == "__main__":
    main()