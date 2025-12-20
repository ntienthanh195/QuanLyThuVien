#include <QtTest>
#include "tst_NgayThang.h"
#include "tst_DocGia.h"
#include "tst_MuonTra.h"

int main(int argc, char *argv[])
{
    int status = 0;
    
    TestNgayThang tc1;
    status |= QTest::qExec(&tc1, argc, argv);
    
    TestDocGia tc2;
    status |= QTest::qExec(&tc2, argc, argv);
    
    TestMuonTra tc3;
    status |= QTest::qExec(&tc3, argc, argv);
    
    return status;
}
