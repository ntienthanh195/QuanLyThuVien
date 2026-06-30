import os
import codecs

def add_bom_to_csv(filename):
    try:
        # Read the content as standard UTF-8
        with open(filename, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # Write it back with 'utf-8-sig' which adds the BOM
        with open(filename, 'w', encoding='utf-8-sig') as f:
            f.write(content)
            
        print(f"Success: Added BOM to {filename}")
    except Exception as e:
        print(f"Error processing {filename}: {e}")

if __name__ == "__main__":
    add_bom_to_csv('Mau_Nhap_Sach.csv')
    add_bom_to_csv('Mau_Nhap_Doc_Gia.csv')
