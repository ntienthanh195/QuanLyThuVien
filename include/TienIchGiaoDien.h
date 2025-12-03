#ifndef TIENICHGIAODIEN_H
#define TIENICHGIAODIEN_H

#include <SFML/Graphics.hpp>
#include <string>
#include "GiaoDienSFML.h"

sf::Text TaoVanBan(const sf::Font& font, const std::string& chuoi, unsigned int coChu, sf::Color mauSac);
void CanGiuaGocVanBan(sf::Text& text);
void CanGiuaGocXVanBan(sf::Text& text);
void VeKhung(sf::RenderWindow& window, float x, float y, float w, float h, const std::string& tieuDe, const sf::Font& font);
void TaoNut(const sf::Font& font, MaUI idNut, float x, float y, float rong, float cao, const std::string& nhan, sf::Color mauNen, sf::Color mauChu);
void TaoInput(const sf::Font& font, MaUI idInput, float x, float y, float rong, float cao, const std::string& label, const std::string& giaTri, const std::string& goiY = "", bool readOnly = false);
std::string WordWrapText(const sf::Font& font, const std::string& text, unsigned int fontSize, float maxWidth);
void XuLyNhapChuoi(sf::Event event, std::string& str, size_t maxLen);

#endif // TIENICHGIAODIEN_H
