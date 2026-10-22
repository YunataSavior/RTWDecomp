#line 2 "E:\\Perforce\\Rome\\Rome,BI-1.5,1.6\\Dev\\barbi_daily\\code\\ui\\ui_message_handler.cpp"
static char THIS_FILE[] = __FILE__;

// The windows that event messages open. (The original's path, which its __FILE__s give.)
#include "ui_message_handler.h"
#include "event_messages.h"
#include "../romans_cmd_line.h"

// Opens the message's window, now or after the others, in place of any it had. Under -ai the window is closed and
// deleted at once, so no message waits for the player.
UIMessageBox* UIMessageHandler::open(EventMessage* message, bool now, int x)
{
    if (!m_4)
        return 0;
#line 830
    UIMessageBox* box = NEW UIMessageBox(message);
    if (box) {
        box->unk_018(UIPoint());
        box->unk_060(!m_8);
        if (message->m_window)
            message->m_window->unk_060(false);
        if (lbl_012CA766) {
            box->unk_b0_174();
            box->unk_058();
#line 850
            CA_DELETE(box);
            box = 0;
        }
        if (now)
            unk_00D1C7FA(box, x);
        else
            m_30.push_back(box);
    } else if (message->m_window) {
        message->m_window->unk_058();
#line 868
        CA_DELETE(message->m_window);
        message->m_window = 0;
    }
    return box;
}
