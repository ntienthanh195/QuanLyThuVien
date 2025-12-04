#!/usr/bin/env python3
"""
Script to create an ICO file from an image for Qt application
"""
from PIL import Image
import sys
import os

def create_icon(input_path, output_path):
    """
    Convert an image to .ico format with multiple sizes
    """
    try:
        # Open the image
        img = Image.open(input_path)
        
        # Convert to RGBA if not already
        if img.mode != 'RGBA':
            img = img.convert('RGBA')
        
        # Create icon with multiple sizes
        sizes = [(16, 16), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)]
        
        # Resize and save as ICO
        icon_sizes = []
        for size in sizes:
            resized = img.resize(size, Image.Resampling.LANCZOS)
            icon_sizes.append(resized)
        
        # Save as ICO with all sizes
        icon_sizes[0].save(
            output_path,
            format='ICO',
            sizes=[(img.width, img.height) for img in icon_sizes]
        )
        
        print(f"Successfully created icon: {output_path}")
        return True
        
    except Exception as e:
        print(f"Error creating icon: {e}")
        return False

if __name__ == "__main__":
    # Đường dẫn đầy đủ
    base_dir = os.path.dirname(os.path.abspath(__file__))
    input_image = os.path.join(base_dir, "resources", "library_icon_source.png")
    output_icon = os.path.join(base_dir, "resources", "app_icon.ico")
    
    if not os.path.exists(input_image):
        print(f"Error: Input image not found: {input_image}")
        print("Please save your icon image as 'resources/library_icon_source.png'")
        sys.exit(1)
    
    if create_icon(input_image, output_icon):
        print("Icon created successfully!")
        print(f"Output: {output_icon}")
    else:
        print("Failed to create icon")
        sys.exit(1)
