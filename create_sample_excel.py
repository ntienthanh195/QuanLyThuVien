import xlsxwriter
import os

def create_sample_files():
    # 1. SAMPLE BOOK FILE
    workbook = xlsxwriter.Workbook('Mau_Nhap_Sach.xlsx')
    worksheet = workbook.add_worksheet()
    
    # Headers
    headers = ['ISBN', 'Tên Sách', 'Tác Giả', 'Năm XB', 'Thể Loại', 'Giá Tiền', 'Số Lượng', 'Nhà Xuất Bản', 'Vị Trí']
    data = [
        ['9786047723456', 'Dế Mèn Phiêu Lưu Ký', 'Tô Hoài', 2020, 'Văn Học', 50000, 10, 'NXB Kim Đồng', 'Ke A1'],
        ['9780061120084', 'To Kill a Mockingbird', 'Harper Lee', 1960, 'Novel', 150000, 5, 'HarperCollins', 'Ke B2'],
        ['9780307277671', 'The Da Vinci Code', 'Dan Brown', 2003, 'Mystery', 120000, 8, 'Doubleday', 'Ke C3']
    ]
    
    # Write Headers
    header_format = workbook.add_format({'bold': True, 'bg_color': '#D7E4BC'})
    for col, header in enumerate(headers):
        worksheet.write(0, col, header, header_format)
        worksheet.set_column(col, col, 20)
        
    # Write Data
    for row_idx, row_data in enumerate(data, start=1):
        for col_idx, cell_data in enumerate(row_data):
            worksheet.write(row_idx, col_idx, cell_data)
            
    workbook.close()
    print("Created 'Mau_Nhap_Sach.xlsx'")

    # 2. SAMPLE READER FILE
    workbook_rd = xlsxwriter.Workbook('Mau_Nhap_Doc_Gia.xlsx')
    worksheet_rd = workbook_rd.add_worksheet()
    
    # Headers
    headers_rd = ['Họ Đệm', 'Tên', 'Giới Tính', 'Ngày Sinh', 'Lớp', 'SĐT', 'Email']
    data_rd = [
        ['Nguyễn Văn', 'An', 'Nam', '01/01/2005', '12A1', '0901234567', 'an.nguyen@email.com'],
        ['Trần Thị', 'Bình', 'Nữ', '15/05/2006', '11B2', '0909888777', 'binh.tran@email.com'],
        ['Lê', 'Cường', 'Nam', '20/11/2004', '12A5', '0912345678', 'cuong.le@email.com']
    ]
    
    # Write Headers
    for col, header in enumerate(headers_rd):
        worksheet_rd.write(0, col, header, header_format)
        worksheet_rd.set_column(col, col, 20)
        
    # Write Data
    for row_idx, row_data in enumerate(data_rd, start=1):
        for col_idx, cell_data in enumerate(row_data):
            worksheet_rd.write(row_idx, col_idx, cell_data)
            
    workbook_rd.close()
    print("Created 'Mau_Nhap_Doc_Gia.xlsx'")

if __name__ == "__main__":
    try:
        create_sample_files()
    except Exception as e:
        print(f"Error: {e}")
