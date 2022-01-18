#include <tempo.h>

class MainApp : public Tempo::App {
private:
    Tempo::FontID m_font_regular;
    Tempo::FontID m_font_italic;
    Tempo::FontID m_font_bold;
    Tempo::FontID m_font_title;
    bool m_open = true;
public:
    virtual ~MainApp() {}

    void InitializationBeforeLoop() override;
    void FrameUpdate() override;
    void BeforeFrameUpdate() override;
};