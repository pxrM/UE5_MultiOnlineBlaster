import os
import chardet

def convert_encoding(root_dir, extensions=('.h', '.cpp')):
    for root, dirs, files in os.walk(root_dir):
        for file in files:
            if file.lower().endswith(extensions):
                filepath = os.path.join(root, file)
                
                # 读取二进制内容
                with open(filepath, 'rb') as f:
                    raw_data = f.read()
                
                # 检测编码
                result = chardet.detect(raw_data)
                original_encoding = result['encoding'] or 'gbk'
                
                # 转换内容（保留原始换行符）
                try:
                    content = raw_data.decode(original_encoding)
                except UnicodeDecodeError:
                    content = raw_data.decode('gbk', errors='replace')
                
                # 关键修改：写入时保持原始换行符
                with open(filepath, 'w', encoding='utf-8', newline='') as f:
                    f.write(content)
                print(f"Fixed: {filepath}")

if __name__ == "__main__":
    target_folder = input("请输入文件夹路径：").strip('"')
    convert_encoding(target_folder)