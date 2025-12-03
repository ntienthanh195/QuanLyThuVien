#ifndef GIAO_DIEN_QUAN_LY_DOC_GIA_H
#define GIAO_DIEN_QUAN_LY_DOC_GIA_H

#include <SFML/Graphics.hpp>
#include "Constants.h"
#include "DocGia.h"

// Ma UI rieng cho man hinh doc gia
enum MaUI_DocGia {
    INPUT_HO = 500,
    INPUT_TEN,
    NUT_PHAI,
    NUT_TT,
    NUT_THEM_DG,
    NUT_SUA_DG,
    NUT_XOA_DG,
    NUT_LAMMOI_DG,
    NUT_QUAYLAI_DG,
};

// Ham khoi tao va ve
void KhoiTaoManHinhDocGia();
void VeManHinhQuanLyDocGia(sf::RenderWindow& window, const sf::Font& font);
void XuLySuKienManHinhDocGia(sf::RenderWindow& window, sf::Event event);
void VeBangDocGia(sf::RenderWindow& window, const sf::Font& font, float yStart);

#endif
