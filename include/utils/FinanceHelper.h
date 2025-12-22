#ifndef FINANCEHELPER_H
#define FINANCEHELPER_H

#include <QString>
#include <QMessageBox>
#include "models/DocGia.h"
#include "utils/LanguageManager.h"
#include "utils/SettingsManager.h"
#include "utils/NgayThang.h"
#include "utils/ToastNotification.h"
#include "utils/EventBus.h"
#include "giao_dien/hop_thoai/HopThoaiThanhToan.h"
#include "models/MuonTra.h"
#include <sstream>

/**
 * @class FinanceHelper
 * @description Centralized controller for Library Finance operations (Payment & Refund).
 */
class FinanceHelper {
public:
    struct PaymentContext {
        long long amount = 0;
        QString reason;
        bool isRefund = false;
        bool canProcess = false;
        QString maSach;
    };

    /**
     * @brief Calculate payment context from Reader DTO
     */
    static PaymentContext getContextFromDto(const ThongTinSachDangMuon_DTO& dto) {
        PaymentContext ctx;
        ctx.maSach = QString::fromStdString(dto.maSach);
        ctx.isRefund = (dto.trangThai == 4); // GD_MAT_CHO_HOAN_TIEN

        if (ctx.isRefund) {
            ctx.amount = dto.tienHoanTra;
            ctx.reason = LanguageManager::instance().getText("MSG_FINANCE_REFUND_REASON").arg(ctx.maSach);
            if (ctx.reason.isEmpty()) ctx.reason = QString("Hoàn phí đền bù (Tìm thấy sách %1)").arg(ctx.maSach);
            ctx.canProcess = (ctx.amount > 0);
        } else {
            // Priority 1: Stored debt (Status 1: Returned, Status 2: Lost)
            ctx.amount = dto.tienPhat;
            
            // Priority 2: Calculate overdue for current loans (Status 0)
            if (ctx.amount <= 0 && dto.trangThai == 0) {
                std::stringstream ss;
                int soNgay = tinhSoNgayQuaHan(dto.ngayMuon, ss);
                if (soNgay > 0) {
                    int finePerDay = SettingsManager::instance().getInt("TIEN_PHAT_MOI_NGAY", 5000);
                    ctx.amount = (long long)soNgay * finePerDay;
                    ctx.reason = QString("Phạt quá hạn (%1 ngày): %2").arg(soNgay).arg(ctx.maSach);
                }
            } else {
                if (dto.trangThai == 2) ctx.reason = QString("Đền bù sách mất: %1").arg(ctx.maSach);
                else ctx.reason = QString("Phạt quá hạn tồn đọng: %1").arg(ctx.maSach);
            }
            
            ctx.canProcess = (ctx.amount > 0 && !dto.daThanhToan);
        }
        return ctx;
    }

    /**
     * @brief Unified flow to execute payment/refund
     */
    static bool processPaymentFlow(PTRDG reader, const PaymentContext& ctx, QWidget* parent) {
        if (!reader || !ctx.canProcess) {
            QMessageBox::information(parent, "Thông Báo", "Giao dịch không hợp lệ hoặc đã hoàn tất.");
            return false;
        }

        bool confirmed = HopThoaiThanhToan::hienThiThanhToan(
            reader, 
            ctx.reason, 
            ctx.amount, 
            ctx.isRefund,
            parent
        );

        if (confirmed) {
            // Call backend (State Machine Update)
            std::string error = ThanhToanPhat(reader, ctx.maSach.toStdString());
            
            if (error.empty()) {
                // Notifications
                QString msg = ctx.isRefund ? "Đã hoàn tiền thành công!" : "Đã thanh toán thành công!";
                ToastNotification::show(msg, ToastNotification::SUCCESS);
                
                // Emit updates
                EventBus::instance().muonTraUpdated(reader->data.MaThe, ctx.maSach);
                EventBus::instance().dataChanged();
                return true;
            } else {
                QMessageBox::critical(parent, "Lỗi Database", QString::fromStdString(error));
                return false;
            }
        }
        return false;
    }
};

#endif // FINANCEHELPER_H
