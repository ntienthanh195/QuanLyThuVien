#ifndef MANHINHQUANLYSACH_H
#define MANHINHQUANLYSACH_H

#include <SFML/Graphics.hpp>

// Khai báo các hàm public cho màn hình sách
void VeManHinhQuanLySach(sf::RenderWindow& window, const sf::Font& font);
void XuLySuKienManHinhSach(sf::RenderWindow& window, sf::Event event);
void KhoiTaoManHinhSach(); 

#endif // MANHINHQUANLYSACH_H