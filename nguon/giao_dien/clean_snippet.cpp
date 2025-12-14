
*/

void WidgetMuonTraSach::xuLyInBatch() {
    if (!docGiaHienTai) {
        // Safety check - clear all queues
        m_batchPrintQueueMa.clear();
        m_batchPrintQueueTen.clear();
        m_batchReturnQueueMa.clear();
        m_batchReturnQueueTen.clear();
        return;
    }

    // Check which operation to print for
    if (m_lastOperationWasBorrow && !m_batchPrintQueueTen.isEmpty()) {
        // Print BORROW receipt
        bool success = PrintHelper::inPhieuMuonTra(docGiaHienTai, m_batchPrintQueueTen, true, this);
        
        // Clear borrow queues
        m_batchPrintQueueMa.clear();
        m_batchPrintQueueTen.clear();
        
    } else if (!m_lastOperationWasBorrow && !m_batchReturnQueueTen.isEmpty()) {
        // Print RETURN receipt
        bool success = PrintHelper::inPhieuMuonTra(docGiaHienTai, m_batchReturnQueueTen, false, this);
        
        // Clear return queues
        m_batchReturnQueueMa.clear();
        m_batchReturnQueueTen.clear();
    }
}
