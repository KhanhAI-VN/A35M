import os
import re
import subprocess
import sys

def remove_comments(text):
    # Biểu thức chính quy (regex) để tìm comment và string
    # Regex này giữ nguyên các chuỗi (strings) để tránh xóa nhầm // hoặc /* bên trong chuỗi
    pattern = re.compile(
        r'(?P<string>"(?:\\.|[^\\"])*"|\'(?:\\.|[^\\\'])*\')|(?P<comment>//[^\n]*|/\*.*?\*/)',
        re.DOTALL
    )
    def replacer(match):
        if match.group('string'):
            return match.group('string') # Nếu là string thì giữ nguyên
        return '' # Nếu là comment thì xóa
    
    return re.sub(pattern, replacer, text)

def process_file(filepath):
    print(f"Processing: {filepath}")
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()
    
    new_content = remove_comments(content)
    
    # Ghi lại file nếu có thay đổi
    if content != new_content:
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(new_content)
        print(f"  -> Đã xóa comment.")
    else:
        print(f"  -> Không có comment nào.")
    
    # Chạy clang-format để format lại file
    try:
        subprocess.run(['clang-format', '-style=Google', '-i', filepath], check=True)
        print(f"  -> Đã format bằng clang-format (chuẩn Google).")
    except subprocess.CalledProcessError as e:
        print(f"  -> Lỗi khi format {filepath}: {e}")
    except FileNotFoundError:
        print("  -> Lỗi: Không tìm thấy lệnh 'clang-format'. Hãy cài đặt clang-format (ví dụ: sudo apt install clang-format).")

def main():
    # Nhận danh sách thư mục/file từ command line, mặc định là các file được chỉ định
    targets = sys.argv[1:]
    if not targets:
        print("Vui lòng truyền vào thư mục hoặc file cần xử lý.")
        print("Ví dụ: python3 remove_comments.py test.c src/ include/")
        return

    for target in targets:
        if os.path.isfile(target):
            if target.endswith('.c') or target.endswith('.h'):
                process_file(target)
        elif os.path.isdir(target):
            for root, _, files in os.walk(target):
                for file in files:
                    if file.endswith('.c') or file.endswith('.h'):
                        process_file(os.path.join(root, file))
        else:
            print(f"Không tìm thấy: {target}")

if __name__ == '__main__':
    main()
