# 📋 TODO - Next Steps

## 🔴 Phase 5: Implement CRUD Dialogs (Priority 1)

### 1. Book Dialogs
- [ ] **AddBookDialog** (`src/views/dialogs/AddBookDialog.cpp`)
  - Form fields: ISBN, Tên sách, Tác giả, Thể loại, Năm XB, Số trang, Số bản sao, Vị trí
  - Validation: ISBN format, năm > 0, số trang > 0
  - Call: `ThemDauSachMoi()` từ QuanLySach.h
  - Signal: `bookAdded()` để refresh table

- [ ] **EditBookDialog** (tương tự AddBookDialog)
  - Pre-fill data từ selected book
  - Call: `CapNhatThongTinDauSach()`
  - Signal: `bookUpdated()`

- [ ] **BookDetailsDialog**
  - QTableWidget hiển thị danh sách bản sao (Mã sách, Trạng thái, Vị trí)
  - Nút "Thanh lý" cho từng bản sao
  - Call: `LayDanhSachBanSaoSapXep()`, `ThanhLyBanSaoTheoMa()`

- [ ] **AddCopyDialog**
  - Spinner cho số lượng (1-100)
  - LineEdit cho vị trí
  - Call: `ThemBanSaoMoi()`

### 2. Reader Dialogs
- [ ] **AddReaderDialog** (`src/views/dialogs/AddReaderDialog.cpp`)
  - Fields: Họ, Tên, Phái (QComboBox), Trạng thái (QCheckBox)
  - Auto-generate mã thẻ
  - Call: `taoDocGia()` từ DocGia.h
  - Signal: `readerAdded()`

- [ ] **EditReaderDialog**
  - Pre-fill data
  - Call: `hieuChinhDocGia()`
  - Signal: `readerUpdated()`

- [ ] **ReaderLoansDialog**
  - QTableWidget: Mã sách, Tên sách, Ngày mượn, Số ngày giữ
  - Call: `LayDSSachDangMuon()`
  - Show overdue status

## 🟡 Phase 6: Implement Reports (Priority 2)

### 3. Report Dialogs
- [ ] **Top10BooksDialog** (`src/views/reports/Top10BooksDialog.cpp`)
  - QTableView: Rank, ISBN, Tên sách, Số lượt mượn
  - Call: `LayTopSach()`
  - Export to TXT option

- [ ] **OverdueReportDialog**
  - QTableView: Mã thẻ, Họ tên, Số ngày quá hạn max
  - Call: `LayDSDocGiaQuaHan()`
  - Highlight severe cases (>30 days)

## 🟢 Phase 7: Polish & Enhancement (Priority 3)

### 4. UI/UX Improvements
- [ ] Add real icons (từ icons8.com hoặc flaticon)
- [ ] Create custom stylesheet (`resources/style.qss`)
- [ ] Add tooltips cho các buttons
- [ ] Implement search suggestions/autocomplete
- [ ] Add keyboard shortcuts (Ctrl+N, Ctrl+F, etc.)
- [ ] Progress bar cho long operations

### 5. Error Handling
- [ ] Better error messages với QMessageBox
- [ ] Input validation với visual feedback (red border)
- [ ] Confirm dialogs cho destructive actions
- [ ] Status bar messages

### 6. Data Management
- [ ] Auto-save on close
- [ ] Backup/Restore functionality
- [ ] Import/Export CSV
- [ ] Settings dialog (font size, theme, etc.)

## 🔵 Phase 8: Testing & Documentation (Priority 4)

### 7. Testing
- [ ] Manual testing tất cả use cases
- [ ] Edge case testing (empty data, max capacity)
- [ ] Performance testing với large datasets
- [ ] Unit tests cho backend functions (optional)

### 8. Documentation
- [ ] User manual trong Help menu
- [ ] Code comments và documentation
- [ ] Video demo/tutorial
- [ ] GitHub README with screenshots

## 📐 Code Template Examples

### Dialog Template:
```cpp
// include/views/dialogs/AddBookDialog.h
class AddBookDialog : public QDialog {
    Q_OBJECT
public:
    explicit AddBookDialog(QWidget *parent = nullptr);
    
signals:
    void bookAdded();
    
private slots:
    void onSaveClicked();
    void onCancelClicked();
    void validateInput();
    
private:
    QLineEdit *isbnEdit, *nameEdit, *authorEdit;
    QSpinBox *yearSpin, *pagesSpin, *copiesSpin;
    QPushButton *btnSave, *btnCancel;
    void setupUI();
    bool validateFields();
};
```

## 🎯 Estimated Time

| Phase | Tasks | Est. Time |
|-------|-------|-----------|
| Phase 5 | 6 dialogs | 4-6 hours |
| Phase 6 | 2 reports | 2-3 hours |
| Phase 7 | Polish | 3-4 hours |
| Phase 8 | Testing | 2-3 hours |
| **Total** | | **11-16 hours** |

## 🚀 How to Continue

1. Create `src/views/dialogs/` folder
2. Start with AddBookDialog (easiest, có backend ready)
3. Test thoroughly
4. Move to next dialog
5. Keep backend unchanged!

---

**Current Status**: ✅ Framework ready, waiting for dialogs  
**Next Action**: Implement AddBookDialog
