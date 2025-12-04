#include "utils/PrintQueueManager.h"
#include <QDebug>

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
