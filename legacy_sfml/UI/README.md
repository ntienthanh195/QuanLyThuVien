README: Hướng Dẫn Kỹ Thuật Giao Diện SFML
Chào mừng các bạn đến với phần giao diện của đồ án Quản Lý Thư Viện. Toàn bộ cửa sổ, nút bấm, văn bản, và tương tác người dùng đều được xây dựng bằng thư viện SFML (Simple and Fast Multimedia Library).

Tài liệu này giải thích cách chúng ta sử dụng SFML, mục đích của các hàm chính, và cách chúng được trừu tượng hóa trong dự án này.

1. Triết Lý Cốt Lõi: Vòng Lặp Chính (The Game Loop)
Không giống như một số framework UI khác, SFML hoạt động dựa trên một Vòng Lặp Chính (thường gọi là "Game Loop"). Đây là một vòng lặp while chạy liên tục hàng chục lần mỗi giây.

Nó nằm ở đâu? Trong file ui/GiaoDienSFML.cpp, bên trong hàm ChayChuongTrinhSFML.

Mỗi vòng lặp (mỗi khung hình) thực hiện 3 bước chính:

Xử lý Sự kiện (Input): XuLySuKienSFML()

Kiểm tra xem người dùng có làm gì không (di chuột, nhấn phím, click,...).

Vẽ (Draw): VeManHinhHienTai()

Xóa toàn bộ màn hình cũ (window.clear(MAU_NEN)).

Vẽ lại tất cả các thành phần (nút, chữ, khung) của màn hình hiện tại từ đầu.

Hiển thị (Display): window.display()

Đẩy những gì vừa vẽ ở bước 2 ra cho người dùng thấy.

Đây là lý do tại sao các hàm Ve... của chúng ta được gọi liên tục.

2. Module Window: Cửa Sổ và Bắt Sự Kiện
Đây là module quản lý cửa sổ và tất cả đầu vào từ người dùng.

sf::RenderWindow (Cửa Sổ)
Là gì: Đây là đối tượng cửa sổ chính của chương trình.

Cách dùng: Được tạo trong main.cpp và truyền đi khắp nơi.

Hàm chính:

window.create(sf::VideoMode(w, h), "Tieu de"): Tạo cửa sổ với kích thước w và h (lấy từ CHIEU_RONG, CHIEU_CAO).

window.pollEvent(event): Cực kỳ quan trọng. Hàm này "hỏi" cửa sổ xem có sự kiện nào vừa xảy ra không (như click, gõ phím). Chúng ta gọi nó liên tục trong XuLySuKienSFML.

window.draw(doiTuong): Vẽ một đối tượng (sẽ nói ở module Graphics) lên bộ đệm (buffer) của cửa sổ.

window.clear() / window.display(): Như đã giải thích ở Vòng Lặp Chính.

sf::Event (Sự Kiện)
Là gì: Một struct chứa thông tin về loại sự kiện và dữ liệu của sự kiện đó.

Cách dùng: Trong XuLySuKienSFML, chúng ta dùng switch (event.type) để phân loại.

Các loại sự kiện chính:

sf::Event::Closed: Người dùng nhấn nút 'X' của cửa sổ.

sf::Event::MouseButtonPressed: Người dùng nhấn chuột.

Dữ liệu: event.mouseButton.x và event.mouseButton.y.

Mục đích: Lấy tọa độ click để truyền vào hàm LayElementTaiToaDo.

sf::Event::TextEntered: Người dùng gõ một ký tự.

Dữ liệu: event.text.unicode.

Mục đích: Lấy ký tự ASCII (ví dụ: 'a', 'b', '1', '2') để thêm vào ô input (trong XuLyTextInput).

sf::Event::KeyPressed: Người dùng nhấn một phím chức năng.

Dữ liệu: event.key.code (ví dụ: sf::Keyboard::Backspace, sf::Keyboard::Enter) và event.key.control (kiểm tra phím Ctrl).

Mục đích: Xử lý xóa ký tự (Backspace) hoặc chuyển ô (Enter/Tab) trong XuLyTextInput.

sf::Event::MouseWheelScrolled: Người dùng cuộn bánh xe chuột.

Dữ liệu: event.mouseWheelScroll.delta (cho biết cuộn lên hay xuống).

Mục đích: Dùng để điều chỉnh biến scrollOffsetYTheLoai trong chế độ xem theo thể loại.

3. Module Graphics: Vẽ Mọi Thứ
Đây là module giúp chúng ta vẽ hình ảnh lên cửa sổ.

sf::Font (Phông chữ)
Là gì: Đối tượng tải và lưu trữ dữ liệu của một file font (ví dụ: .ttf).

Cách dùng: Bắt buộc phải font.loadFromFile(...) thành công trong main.cpp (hoặc KhoiTaoGiaoDienSFML). Sau đó, ta truyền font này cho mọi hàm cần vẽ chữ.

sf::Text (Văn bản)
Là gì: Một đối tượng có thể vẽ được (Drawable), dùng để hiển thị chữ.

Cách dùng:

Tạo: sf::Text myText;

Thiết lập: myText.setFont(font);, myText.setString("Xin chao");, myText.setCharacterSize(18);, myText.setFillColor(MAU_CHU);, myText.setPosition(x, y);

Vẽ: window.draw(myText);

Trong dự án: Hàm TaoVanBan (trong TienIchGiaoDien.cpp) đã làm tất cả các bước thiết lập này hộ chúng ta.

sf::RectangleShape (Hình chữ nhật)
Là gì: Đối tượng có thể vẽ được (Drawable), dùng để vẽ hình chữ nhật.

Cách dùng: Đây là "viên gạch" xây dựng nên toàn bộ giao diện của mình (nền nút, nền ô input, nền khung, đường kẻ, thanh cuộn).

Tạo: sf::RectangleShape myRect(sf::Vector2f(150, 40)); (Rộng 150, Cao 40)

Thiết lập: myRect.setPosition(x, y);, myRect.setFillColor(MAU_NEN_NUT);, myRect.setOutlineColor(MAU_VIEN);, myRect.setOutlineThickness(1.f);

Vẽ: window.draw(myRect);

Trong dự án: Các hàm VeKhung, TaoNut, TaoInput đều tự động tạo và thiết lập sf::RectangleShape cho chúng ta.

sf::Color (Màu sắc)
Là gì: Một struct lưu 4 giá trị (R, G, B, A).

Cách dùng: Chúng ta định nghĩa sẵn các màu (ví dụ: const sf::Color MAU_NEN(30, 30, 45);) trong GiaoDienSFML.h để dùng chung.

4. Module System: Công Cụ Hỗ Trợ
sf::Clock (Đồng hồ)
Là gì: Một đồng hồ bấm giờ đơn giản.

Cách dùng: Chúng ta dùng nó cho chức năng double-click trong ManHinhQuanLySach.cpp.

Khi click lần 1: doubleClickClock.restart(); (Bấm nút reset đồng hồ).

Khi click lần 2: float thoiGian = doubleClickClock.getElapsedTime().asSeconds(); (Xem đồng hồ chỉ mấy giây).

if (thoiGian < THOI_GIAN_DOUBLE_CLICK) { ... } (Nếu thời gian trôi qua đủ nhỏ -> đây là double-click).

sf::Vector2f (Vector 2D)
Là gì: Một struct đơn giản để lưu 2 giá trị float (là x và y).

Cách dùng: Dùng mọi lúc khi cần đặt kích thước (setSize) hoặc vị trí (setPosition).

myRect.setSize(sf::Vector2f(rong, cao));

5. Kiến Trúc Dự Án (Cách chúng ta "gói" SFML)
Đây là phần quan trọng nhất để hiểu dự án.

TienIchGiaoDien.cpp (Hộp Công Cụ)
Chúng ta không dùng sf::Text hay sf::RectangleShape trực tiếp ở các màn hình, mà dùng các hàm "gói" này:

VeKhung(...): Vẽ trực tiếp một khung và tiêu đề lên window.

TaoNut(...): Không vẽ. Chỉ tạo 1 UIElement (chứa sf::RectangleShape và sf::Text) và lưu vào mảng toàn cục cacElement.

TaoInput(...): Không vẽ. Chỉ tạo 1 UIElement (chứa 3 thứ: hình hộp, nhãn, và chữ bên trong) và lưu vào mảng cacElement.

GiaoDienSFML.cpp (Bộ Điều Phối)
Biến toàn cục: UIElement cacElement[SO_ELEMENT_TOI_DA] và soLuongElement.

Hàm VeManHinhHienTai (Giả định, hoặc logic trong ChayChuongTrinhSFML):

Reset soLuongElement = 0;.

Gọi hàm vẽ của màn hình con (ví dụ: VeManHinhQuanLySach(window, font)).

Hàm con này sẽ gọi TaoNut, TaoInput... để nhét các UIElement vào mảng cacElement.

Sau khi hàm con chạy xong, một vòng lặp for sẽ duyệt mảng cacElement (từ 0 đến soLuongElement) và window.draw() từng cái một.

Hàm LayElementTaiToaDo(x, y):

Duyệt mảng cacElement (từ 0 đến soLuongElement).

Kiểm tra cacElement[i].hinhDang.getGlobalBounds().contains(x, y).

Nếu đúng, trả về cacElement[i].id (ví dụ: NUT_TIM).

ManHinhQuanLySach.cpp (Màn hình ví dụ)
Hàm Ve... (ví dụ VeBangSach): Chỉ gọi TaoNut(...), TaoInput(...) để đăng ký các thành phần cần vẽ.

Hàm XuLySuKien...:

Nhận event từ GiaoDienSFML.cpp.

Nếu là click chuột, gọi MaUI id = LayElementTaiToaDo(...).

Dùng switch (id) để xử lý (ví dụ: case NUT_TIM: ThucHienTimKiemNoiBo(); break;).

6. Kỹ Thuật Nâng Cao: Vùng Cuộn (Trong ManHinhQuanLySach.cpp)
Để xử lý danh sách "Xem theo Thể Loại" bị dài, chúng ta dùng sf::RenderTexture (Tờ Giấy Vẽ Ảo).

Tính Tổng Chiều Cao: Tính toán totalContentHeightTheLoai (chiều cao thật của toàn bộ danh sách).

Tạo Texture: Đảm bảo theLoaiRenderTexture có kích thước contentWidth x totalContentHeightTheLoai.

Vẽ vào Texture: Dùng theLoaiRenderTexture.draw(...) để vẽ toàn bộ danh sách lên "tờ giấy" ảo này (tọa độ từ 0).

Tạo Sprite: Tạo sf::Sprite từ theLoaiRenderTexture.getTexture().

Cắt Sprite: Dùng sprite.setTextureRect(sf::IntRect(0, scrollOffsetYTheLoai, width, height)) để cắt ra một cửa sổ nhìn. scrollOffsetYTheLoai chính là vị trí cuộn của chúng ta.

Vẽ Sprite: window.draw(contentSprite) để vẽ phần đã cắt lên màn hình chính.

Vẽ Thanh Cuộn: Vẽ thanh cuộn (sf::RectangleShape) bên cạnh dựa trên tỷ lệ scrollOffsetYTheLoai / maxScroll.