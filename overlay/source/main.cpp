#define TESLA_INIT_IMPL
#include <tesla.hpp>

class MainGui : public tsl::Gui {
public:
    tsl::elm::Element* createUI() override {
        auto frame = new tsl::elm::OverlayFrame("Translate", "v0.1");
        auto list  = new tsl::elm::List();

        auto item = new tsl::elm::ListItem("Xin chào Switch!");
        item->setClickListener([item](u64 keys) {
            if (keys & HidNpadButton_A) {
                item->setText("Đã bấm A ✓");
                return true;
            }
            return false;
        });
        list->addItem(item);

        // Bảng kiểm tra font: toàn bộ chữ cái tiếng Việt kèm 5 dấu thanh
        static const char* lower[] = {
            "a à á ả ã ạ", "ă ằ ắ ẳ ẵ ặ", "â ầ ấ ẩ ẫ ậ",
            "e è é ẻ ẽ ẹ", "ê ề ế ể ễ ệ",
            "i ì í ỉ ĩ ị",
            "o ò ó ỏ õ ọ", "ô ồ ố ổ ỗ ộ", "ơ ờ ớ ở ỡ ợ",
            "u ù ú ủ ũ ụ", "ư ừ ứ ử ữ ự",
            "y ỳ ý ỷ ỹ ỵ",
        };
        static const char* upper[] = {
            "A À Á Ả Ã Ạ", "Ă Ằ Ắ Ẳ Ẵ Ặ", "Â Ầ Ấ Ẩ Ẫ Ậ",
            "E È É Ẻ Ẽ Ẹ", "Ê Ề Ế Ể Ễ Ệ",
            "I Ì Í Ỉ Ĩ Ị",
            "O Ò Ó Ỏ Õ Ọ", "Ô Ồ Ố Ổ Ỗ Ộ", "Ơ Ờ Ớ Ở Ỡ Ợ",
            "U Ù Ú Ủ Ũ Ụ", "Ư Ừ Ứ Ử Ữ Ự",
            "Y Ỳ Ý Ỷ Ỹ Ỵ",
        };

        list->addItem(new tsl::elm::CategoryHeader("Bảng chữ cái"));
        list->addItem(new tsl::elm::ListItem("a ă â b c d đ e ê g h i k"));
        list->addItem(new tsl::elm::ListItem("l m n o ô ơ p q r s t u ư v x y"));
        list->addItem(new tsl::elm::ListItem("A Ă Â B C D Đ E Ê G H I K"));
        list->addItem(new tsl::elm::ListItem("L M N O Ô Ơ P Q R S T U Ư V X Y"));

        list->addItem(new tsl::elm::CategoryHeader("Chữ thường có dấu"));
        for (auto row : lower)
            list->addItem(new tsl::elm::ListItem(row));

        list->addItem(new tsl::elm::CategoryHeader("Chữ hoa có dấu"));
        for (auto row : upper)
            list->addItem(new tsl::elm::ListItem(row));

        list->addItem(new tsl::elm::CategoryHeader("Câu mẫu & ký hiệu"));
        list->addItem(new tsl::elm::ListItem("Tiếng Việt có dấu đầy đủ"));
        list->addItem(new tsl::elm::ListItem("Người ướt đẫm, lượm quả ổi"));
        list->addItem(new tsl::elm::ListItem("0123456789 ✓ ✗ … « » “ ”"));

        frame->setContent(list);
        return frame;
    }
};

class TranslateOverlay : public tsl::Overlay {
public:
    void initServices() override {}
    void exitServices() override {}
    std::unique_ptr<tsl::Gui> loadInitialGui() override {
        return initially<MainGui>();
    }
};

int main(int argc, char** argv) {
    return tsl::loop<TranslateOverlay>(argc, argv);
}