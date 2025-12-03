#ifndef GIAO_DIEN_MUON_TRA_H
#define GIAO_DIEN_MUON_TRA_H

#include <SFML/Graphics.hpp>
#include "Constants.h"
#include "DocGia.h"
#include "NgayThang.h"

// All UI IDs are now defined in GiaoDienSFML.h
// No need for additional enum here

// Ham khoi tao va ve
void KhoiTaoManHinhMuonTra();
void VeManHinhMuonTra(sf::RenderWindow& window, const sf::Font& font);
void XuLySuKienManHinhMuonTra(sf::RenderWindow& window, sf::Event event);

#endif
