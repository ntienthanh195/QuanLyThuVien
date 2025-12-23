#ifndef TST_DOCGIA_H
#define TST_DOCGIA_H

#include <QtTest>
#include "models/DocGia.h"

class TestDocGia : public QObject
{
    Q_OBJECT

private:
   PTRDG root = nullptr;

private slots:
    void init() {
        root = nullptr;
    }
    
    void cleanup() {
        GiaiPhongCay(root);
    }

    void testTaoDocGia() {
        PTRDG node = taoDocGia("Nguyen", "Van A", true, 1, 100);
        QVERIFY(node != nullptr);
        QCOMPARE(node->data.MaThe, 100);
        QCOMPARE(node->data.Ho, std::string("Nguyen"));
        // Verify default extended fields
        QCOMPARE(node->data.LoaiDocGia, 0); 
    }

    void testInsertDocGia() {
        // Insert unbalanced to check AVL balancing
        PTRDG p1 = taoDocGia("A", "One", true, 1, 10);
        PTRDG p2 = taoDocGia("A", "Two", true, 1, 20);
        PTRDG p3 = taoDocGia("A", "Three", true, 1, 30);
        
        // Disable DB sync for unit test (using mock DB anyway, but Logic might skip if false)
        // logic: InsertDocGia(root, node, syncDB)
        InsertDocGia(root, p1, false);
        InsertDocGia(root, p2, false);
        InsertDocGia(root, p3, false);
        
        // Tree should be balanced. 20 should be root.
        QVERIFY(root != nullptr);
        QCOMPARE(root->data.MaThe, 20); // Rotation happened
        QCOMPARE(root->left->data.MaThe, 10);
        QCOMPARE(root->right->data.MaThe, 30);
    }
    
    void testTimDocGia() {
         PTRDG p1 = taoDocGia("A", "One", true, 1, 50);
         InsertDocGia(root, p1, false);
         
         PTRDG found = timDocGia(root, 50);
         QVERIFY(found != nullptr);
         
         PTRDG notFound = timDocGia(root, 999);
         QVERIFY(notFound == nullptr);
    }
};

#endif // TST_DOCGIA_H
