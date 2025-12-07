#include "utils/PrintQueueManager.h"
#include <QDebug>
#include <QPrinter>
#include <QPrintDialog>
#include <QTextDocument>
#include <QTextCursor>
#include <QFile>
#include <QDateTime>
#include <QCoreApplication>
#include <QDir>
#include "utils/PrintHelper.h"

PrintQueueManager* PrintQueueManager::s_instance = nullptr;

PrintQueueManager::PrintQueueManager(QObject *parent)
    : QObject(parent)
{
}

PrintQueueManager::~PrintQueueManager()
{
}

PrintQueueManager* PrintQueueManager::instance() {
    if (!s_instance) {
        s_instance = new PrintQueueManager();
    }
    return s_instance;
}

bool PrintQueueManager::them(PTRDG docGia) {
    if (!docGia) {
        qWarning() << "PrintQueueManager::them - docGia is null";
        return false;
    }
    
    // Kiểm tra đã tồn tại chưa
    if (daTonTai(docGia->data.MaThe)) {
        qDebug() << "Độc giả" << docGia->data.MaThe << "đã có trong hàng đợi";
        return false;
    }
    
    m_danhSach.append(docGia);
    emit hangDoiThayDoi();
    
    qDebug() << "Đã thêm độc giả" << docGia->data.MaThe << "vào hàng đợi. Tổng:" << m_danhSach.size();
    return true;
}

void PrintQueueManager::xoa(int index) {
    if (index >= 0 && index < m_danhSach.size()) {
        PTRDG dg = m_danhSach[index];
        m_danhSach.removeAt(index);
        emit hangDoiThayDoi();
        
        qDebug() << "Đã xóa độc giả" << dg->data.MaThe << "khỏi hàng đợi. Còn lại:" << m_danhSach.size();
    }
}

void PrintQueueManager::xoaTatCa() {
    m_danhSach.clear();
    emit hangDoiThayDoi();
    qDebug() << "Đã xóa toàn bộ hàng đợi in";
}

QList<PTRDG> PrintQueueManager::layDanhSach() const {
    return m_danhSach;
}

int PrintQueueManager::soLuong() const {
    return m_danhSach.size();
}

bool PrintQueueManager::daTonTai(int maThe) const {
    for (PTRDG dg : m_danhSach) {
        if (dg && dg->data.MaThe == maThe) {
            return true;
        }
    }
    return false;
}

// Helper: build HTML string for the current queue
static QString buildHtmlForQueue(const QList<PTRDG> &list, const QString &title) {
    QString html;
    html += "<!DOCTYPE html><html><head><meta charset=\"utf-8\">";
    html += "<style> body { font-family: 'Segoe UI', Tahoma, Arial; color: #222; margin: 20px; } ";
    html += "table { border-collapse: collapse; width: 100%; margin-bottom: 12px; } ";
    html += "th, td { border: 1px solid #DDD; padding: 8px; text-align: left; } ";
    html += "th { background: #f5f7fa; font-weight: 700; } ";
    html += ".card { width: 100%; padding: 12px; box-sizing: border-box; border: 1px solid #EEE; border-radius: 6px; margin-bottom: 20px; } ";
    html += ".meta { color: #666; font-size: 12px; margin-bottom: 8px; }";
    html += "</style></head><body>";

    html += QString("<h2>%1</h2>").arg(title.toHtmlEscaped());
    html += QString("<div class=\"meta\">Xuất: %1</div>").arg(QDateTime::currentDateTime().toString());

    for (PTRDG dg : list) {
        if (!dg) continue;
        QString ho = QString::fromStdString(dg->data.Ho).toHtmlEscaped();
        QString ten = QString::fromStdString(dg->data.Ten).toHtmlEscaped();
        QString fullName = ho + " " + ten;
        QString phai = dg->data.Phai ? "Nữ" : "Nam";
        QString trangThai = dg->data.TrangThai == 1 ? "Hoạt động" : "Bị khóa";

        html += "<div class=\"card\">";
        html += QString("<h3>Độc giả: %1</h3>").arg(fullName);
        html += "<table>";
        html += "<tr><th>Mã thẻ</th><th>Họ</th><th>Tên</th><th>Giới tính</th><th>Trạng thái</th><th>Số đang mượn</th></tr>";
        html += QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td><td>%6</td></tr>")
                    .arg(dg->data.MaThe)
                    .arg(QString::fromStdString(dg->data.Ho).toHtmlEscaped())
                    .arg(QString::fromStdString(dg->data.Ten).toHtmlEscaped())
                    .arg(phai)
                    .arg(trangThai)
                    .arg(dg->data.soSachDangMuon);
        html += "</table>";

        // List borrowed books (if any)
        if (dg->data.dsmt != nullptr) {
            html += "<div><strong>Giao dịch mượn/trả:</strong>";
            html += "<table><tr><th>Mã sách</th><th>Ngày mượn</th><th>Ngày trả</th><th>Trạng thái</th></tr>";
            MUONTRA mt = dg->data.dsmt;
            int row = 0;
            while (mt && row < 1000) {
                QString maSach = QString::fromStdString(mt->data.banSaoSach ? mt->data.banSaoSach->maSach : "");
                QString ngayMuon = QString::fromStdString(mt->data.NgayMuon);
                QString ngayTra = QString::fromStdString(mt->data.NgayTra);
                QString tt = (mt->data.TrangThai == 0) ? "Đang mượn" : (mt->data.TrangThai == 1 ? "Đã trả" : "Mất");
                html += QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td></tr>")
                        .arg(maSach.toHtmlEscaped())
                        .arg(ngayMuon.toHtmlEscaped())
                        .arg(ngayTra.toHtmlEscaped())
                        .arg(tt);
                mt = mt->next;
                ++row;
            }
            html += "</table></div>";
        }

        html += "</div>"; // card
        html += "<div style=\"page-break-after: always;\"></div>"; // each card on its own page
    }

    html += "</body></html>";
    return html;
}

void PrintQueueManager::inHangDoi(QWidget *parent) {
    if (m_danhSach.isEmpty()) {
        qDebug() << "Hàng đợi trống, không in.";
        return;
    }

    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog dlg(&printer, parent);
    dlg.setWindowTitle("In danh sách độc giả");
    if (dlg.exec() != QDialog::Accepted) return;

    QString html = buildHtmlForQueue(m_danhSach, "Danh sách độc giả cần in");

    QTextDocument doc;
    doc.setHtml(html);
    doc.setPageSize(printer.pageRect(QPrinter::Point).size());
    doc.print(&printer);
}

bool PrintQueueManager::xuatHangDoiThanhPdf(const QString &filePath, const QString &title) {
    if (m_danhSach.isEmpty()) return false;
    if (filePath.isEmpty()) return false;

    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filePath);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setFullPage(false);
    printer.setResolution(300);

    QString html = buildHtmlForQueue(m_danhSach, title);
    QTextDocument doc;
    doc.setHtml(html);
    doc.setPageSize(printer.pageRect(QPrinter::Point).size());

    // Attempt to write PDF
    doc.print(&printer);
    return QFile::exists(filePath);
}

bool PrintQueueManager::xuatSamplePdf(ExportLayout layout) {
    if (m_danhSach.isEmpty()) return false;

    QString filePath = QCoreApplication::applicationDirPath() + "/data/sample_print.pdf";

    // Build document according to chosen layout using PrintHelper
    QTextDocument doc;
    if (layout == ExportLayout::ReportA4) {
        // Build a report table using PrintHelper: create HTML via taoHTMLDanhSach
        // Reuse PrintHelper::inDanhSachDocGia by constructing a temporary root? Simpler: call PrintHelper::inDanhSachDocGia requires root.
        // Instead, use taoHTMLDanhSach via PrintHelper private helpers not accessible; so we use existing public APIs.
        // Create a temporary container: call inDanhSachDocGia would open print dialog. Instead build HTML table manually.
        QString bang = "<table border='1' cellpadding='5' cellspacing='0' style='width:100%; border-collapse:collapse;'>"
                       "<tr style='background:#f0f0f0;'><th>Mã thẻ</th><th>Họ tên</th><th>Giới tính</th><th>Trạng thái</th></tr>";
        for (PTRDG dg : m_danhSach) {
            if (!dg) continue;
            QString hoTen = QString::fromStdString(dg->data.Ho + " " + dg->data.Ten);
            QString phai = dg->data.Phai ? "Nữ" : "Nam";
            QString trangThai = dg->data.TrangThai == 1 ? "Hoạt động" : "Khóa";
            bang += QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td></tr>")
                    .arg(dg->data.MaThe).arg(hoTen).arg(phai).arg(trangThai);
        }
        bang += "</table>";
        QString html = QString("<html><head><meta charset='utf-8'><style>body{font-family:Arial;}th{background:#f0f0f0;}</style></head><body><h2>Sample Report</h2>%1</body></html>").arg(bang);
        doc.setHtml(html);
    } else {
        // CardSize layout: reuse buildHtmlForQueue with a compact title
        QString html = buildHtmlForQueue(m_danhSach, "Sample Cards");
        doc.setHtml(html);
    }

    // Use PrintHelper xuatPDF to save
    bool ok = false;
    // Ensure directory exists
    QDir dir(QCoreApplication::applicationDirPath() + "/data");
    if (!dir.exists()) dir.mkpath(".");

    ok = PrintHelper::xuatPDF(&doc, filePath);
    if (ok) {
        qDebug() << "Sample PDF created:" << filePath;
    } else {
        qWarning() << "Failed to create sample PDF";
    }
    return ok;
}
