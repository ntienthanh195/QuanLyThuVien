#!/usr/bin/env python3
"""
Script để tạo icon từ hình ảnh và đặt vào resources/app_icon.ico
"""
from PIL import Image, ImageDraw, ImageFont
import os

def create_library_icon(output_path):
    """
    Tạo icon thư viện với thiết kế giống hình đã gửi
    """
    try:
        # Tạo hình ảnh 512x512 với nền trong suốt
        size = 512
        img = Image.new('RGBA', (size, size), (0, 0, 0, 0))
        draw = ImageDraw.Draw(img)
        
        # Màu sắc phong cách cổ điển
        bg_color = (237, 224, 186, 255)  # Màu giấy cũ
        book_color = (139, 69, 19, 255)  # Màu nâu đậm cho sách
        gold_color = (218, 165, 32, 255)  # Màu vàng
        accent_color = (160, 82, 45, 255)  # Màu nâu đỏ
        
        # Vẽ nền tròn
        margin = 20
        draw.ellipse([margin, margin, size-margin, size-margin], fill=bg_color)
        
        # Vẽ cuộn giấy (bên trái)
        scroll_x = 120
        scroll_y = 150
        scroll_width = 100
        scroll_height = 250
        
        # Phần cuộn giấy
        draw.rectangle([scroll_x, scroll_y, scroll_x + scroll_width, scroll_y + scroll_height], 
                      fill=(245, 235, 210, 255), outline=accent_color, width=3)
        
        # Đầu cuộn trên
        draw.ellipse([scroll_x - 10, scroll_y - 20, scroll_x + scroll_width + 10, scroll_y + 10],
                    fill=gold_color, outline=accent_color, width=2)
        
        # Đầu cuộn dưới  
        draw.ellipse([scroll_x - 10, scroll_y + scroll_height - 10, scroll_x + scroll_width + 10, 
                      scroll_y + scroll_height + 20], fill=gold_color, outline=accent_color, width=2)
        
        # Vẽ sách (bên phải, hơi chồng lên cuộn giấy)
        book_x = 220
        book_y = 140
        book_width = 180
        book_height = 240
        
        # Thân sách chính
        draw.rectangle([book_x, book_y, book_x + book_width, book_y + book_height],
                      fill=book_color, outline=accent_color, width=3)
        
        # Viền trang trí vàng
        border_margin = 20
        draw.rectangle([book_x + border_margin, book_y + border_margin,
                       book_x + book_width - border_margin, book_y + book_height - border_margin],
                      outline=gold_color, width=3)
        
        # Vẽ lá bồ đề ở giữa sách (biểu tượng tri thức)
        leaf_center_x = book_x + book_width // 2
        leaf_center_y = book_y + book_height // 2
        
        # Hình lá đơn giản
        leaf_points = [
            (leaf_center_x, leaf_center_y - 50),  # Đỉnh
            (leaf_center_x + 30, leaf_center_y - 20),
            (leaf_center_x + 35, leaf_center_y + 20),
            (leaf_center_x, leaf_center_y + 50),  # Đáy
            (leaf_center_x - 35, leaf_center_y + 20),
            (leaf_center_x - 30, leaf_center_y - 20),
        ]
        draw.polygon(leaf_points, fill=gold_color, outline=(184, 134, 11, 255), width=2)
        
        # Gân lá
        draw.line([(leaf_center_x, leaf_center_y - 50), (leaf_center_x, leaf_center_y + 50)],
                 fill=(184, 134, 11, 255), width=2)
        
        # Vẽ dấu bookmark
        bookmark_x = book_x + book_width - 40
        bookmark_y = book_y - 15
        draw.polygon([
            (bookmark_x, bookmark_y),
            (bookmark_x + 25, bookmark_y),
            (bookmark_x + 25, bookmark_y + 80),
            (bookmark_x + 12, bookmark_y + 65),
            (bookmark_x, bookmark_y + 80)
        ], fill=(178, 34, 34, 255), outline=accent_color, width=2)
        
        # Tạo các kích thước icon khác nhau
        sizes = [(256, 256), (128, 128), (64, 64), (48, 48), (32, 32), (16, 16)]
        images = []
        
        for icon_size in sizes:
            resized = img.resize(icon_size, Image.Resampling.LANCZOS)
            images.append(resized)
        
        # Lưu thành file ICO
        images[0].save(output_path, format='ICO', sizes=[size for size in sizes])
        
        print(f"✓ Icon đã được tạo thành công: {output_path}")
        print(f"  Kích thước: {sizes}")
        return True
        
    except Exception as e:
        print(f"✗ Lỗi khi tạo icon: {e}")
        import traceback
        traceback.print_exc()
        return False

if __name__ == "__main__":
    base_dir = os.path.dirname(os.path.abspath(__file__))
    output_icon = os.path.join(base_dir, "resources", "app_icon.ico")
    
    # Tạo thư mục resources nếu chưa có
    os.makedirs(os.path.join(base_dir, "resources"), exist_ok=True)
    
    if create_library_icon(output_icon):
        print("\n✓ Hoàn tất! Bây giờ bạn có thể build lại ứng dụng.")
        print(f"  Icon location: {output_icon}")
    else:
        print("\n✗ Không thể tạo icon")
