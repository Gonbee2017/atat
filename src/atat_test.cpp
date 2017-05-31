#include<algorithm>
#include"atat.h"
#include<CppUTest/CommandLineTestRunner.h>
#include<iomanip>
#include<iostream>
#include<memory>
#include<string>
#include<vector>
#include<windows.h>

#define TEST_PRINT(x) cout<<#x<<":"<<(x)<<endl

using namespace atat;
using namespace std;

class call
{
public:
    template<class...ARGUMENTS>
    call(const string&name_,ARGUMENTS&&...arguments_);
    const string&arguments() const;
    const string&name() const;
    bool operator!=(const call&rhs) const;
    bool operator==(const call&rhs) const;
private:
    string arguments_;
    string name_;
};

class history
{
public:
    vector<call>&calls();
    size_t number_of(const string&name) const;
private:
    vector<call> calls_;
};

ostream&operator<<(ostream&os,const call&call_);
void sand_box(function<void()> block);
template<class VALUE> SimpleString StringFrom(const VALUE&value);

template<class...ARGUMENTS>
call::call(const string&name_,ARGUMENTS&&...arguments_):
    name_(name_),arguments_(describe_with(",",arguments_...)) {}

const string&call::arguments() const {return arguments_;}

const string&call::name() const {return name_;}

bool call::operator!=(const call&rhs) const {return !(*this==rhs);}

bool call::operator==(const call&rhs) const
{return name_==rhs.name_&&arguments_==rhs.arguments_;}

vector<call>&history::calls() {return calls_;}

size_t history::number_of(const string&name) const
{
    return count_if
    (
        calls_.begin(),
        calls_.end(),
        [&name] (const call&call_)->bool {return call_.name()==name;}
    );
}

ostream&operator<<(ostream&os,const call&call_)
{return os<<describe_with(":",call_.name(),call_.arguments());}

void sand_box(function<void()> block)
{
    block();
    Context::instance().reset();
    atat::CloseHandle=nullptr;
    atat::CreateEvent=nullptr;
    atat::FindWindowW=nullptr;
    atat::GetDoubleClickTime=nullptr;
    atat::GetLastError=nullptr;
    atat::GetSystemMetrics=nullptr;
    atat::GetWindowRect=nullptr;
    atat::SendInput=nullptr;
    atat::SetConsoleCtrlHandler=nullptr;
    atat::SetEvent=nullptr;
    atat::WaitForSingleObject=nullptr;
}

template<class VALUE> SimpleString StringFrom(const VALUE&value)
{
    ostringstream oss;
    oss<<value;
    return oss.str().c_str();
}

TEST_GROUP(Command) {};

TEST(Command,construct)
{
    sand_box([] ()
    {
        auto l=make_shared<Line>("key down A");
        auto kdc=make_shared<KeyDownCommand>(l);
        POINTERS_EQUAL(l.get(),kdc->line());
    });
}

TEST_GROUP(Context) {};

TEST(Context,construct)
{
    sand_box([] ()
    {
        history h;
        atat::CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE
            {
                h.calls().push_back(call
                (
                    "CreateEvent",
                    lpEventAttributes,
                    bManualReset,
                    bInitialState,
                    lpName
                ));
                return (HANDLE)0x12;
            };
        atat::CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        auto c=make_shared<Context>();
        CHECK_EQUAL((HANDLE)0x12,c->abortedEvent());
        CHECK_EQUAL(1,c->frames().size());
        CHECK_EQUAL(0,c->frames().back().counter);
        CHECK_EQUAL(0,c->frames().back().entry);
        CHECK_EQUAL(0,c->frames().back().number);
        CHECK_EQUAL(0,c->index());
        CHECK(c->properties().empty());
        CHECK_EQUAL(1,h.calls().size());
        CHECK_EQUAL
        (call("CreateEvent",NULL,TRUE,FALSE,(LPCTSTR)NULL),h.calls().at(0));
    });
}

TEST(Context,destruct)
{
    sand_box([] ()
    {
        history h;
        atat::CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        atat::CloseHandle=
            [&] (HANDLE hObject)->BOOL
            {
                h.calls().push_back(call("CloseHandle",hObject));
                return TRUE;
            };
        make_shared<Context>();
        CHECK_EQUAL(1,h.calls().size());
        CHECK_EQUAL(call("CloseHandle",(HANDLE)0x12),h.calls().at(0));
    });
}

TEST(Context,access)
{
    sand_box([] ()
    {
        atat::CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        atat::CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        auto c=make_shared<Context>();
        c->frames().push_back({10,20,30});
        CHECK_EQUAL(2,c->frames().size());
        CHECK_EQUAL(10,c->frames().back().counter);
        CHECK_EQUAL(20,c->frames().back().entry);
        CHECK_EQUAL(30,c->frames().back().number);
        c->index()=100;
        CHECK_EQUAL(100,c->index());
        c->properties()=map<string,string>({{"abc","ABC"},{"def","DEF"}});
        CHECK_EQUAL(2,c->properties().size());
        CHECK_EQUAL("ABC",c->properties().at("abc"));
        CHECK_EQUAL("DEF",c->properties().at("def"));
        Context::instance()=c;
        POINTERS_EQUAL(c.get(),Context::instance().get());
    });
}

TEST_GROUP(KeyCommand) {};

TEST(KeyCommand,construct)
{
    sand_box([] ()
    {
        auto l=make_shared<Line>("key");
        try
        {
            auto kdc=make_shared<KeyDownCommand>(l);
            FAIL("Don't pass here.");
        } catch(const runtime_error&e)
        {STRCMP_EQUAL("line:'key':wrong number of tokens",e.what());}
    });
    sand_box([] ()
    {
        auto l=make_shared<Line>("key down A B");
        try
        {
            auto kdc=make_shared<KeyDownCommand>(l);
            FAIL("Don't pass here.");
        } catch(const runtime_error&e)
        {
            STRCMP_EQUAL
            ("line:'key down A B':wrong number of tokens",e.what());
        }
    });
    sand_box([] ()
    {
        auto l=make_shared<Line>("key down HOGE");
        try
        {
            auto kdc=make_shared<KeyDownCommand>(l);
            FAIL("Don't pass here.");
        } catch(const runtime_error&e)
        {STRCMP_EQUAL("key:'HOGE':unknown",e.what());}
    });
    sand_box([] ()
    {
        auto l=make_shared<Line>("key down A");
        auto kdc=make_shared<KeyDownCommand>(l);
    });
    sand_box([] ()
    {
        auto l=make_shared<Line>("key down  a\t");
        auto kdc=make_shared<KeyDownCommand>(l);
    });
    sand_box([] ()
    {
        auto l=make_shared<Line>("key down B");
        auto kdc=make_shared<KeyDownCommand>(l);
    });
}

TEST(KeyCommand,send)
{
    sand_box([] ()
    {
        history h;
        atat::CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        atat::CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        atat::SendInput=
            [&] (UINT nInputs,LPINPUT pInputs,int cbSize)->UINT
            {
                h.calls().push_back(call
                (
                    "SendInput",
                    nInputs,
                    pInputs->type,
                    pInputs->ki.wScan,
                    pInputs->ki.dwFlags,
                    pInputs->ki.time,
                    pInputs->ki.dwExtraInfo,
                    cbSize
                ));
                return 1;
            };
        atat::GetDoubleClickTime=[&] ()->UINT {return 100;};
        atat::WaitForSingleObject=
            [&] (HANDLE hHandle,DWORD dwMilliseconds)->DWORD
            {return WAIT_TIMEOUT;};
        Context::instance()=make_shared<Context>();
        auto l=make_shared<Line>("key down A");
        auto kdc=make_shared<KeyDownCommand>(l);
        kdc->execute();
        CHECK_EQUAL(1,h.calls().size());
        CHECK_EQUAL
        (
            call
            (
                "SendInput",
                1,
                INPUT_KEYBOARD,
                0x1E,
                KEYEVENTF_SCANCODE,
                0,
                0,
                sizeof(INPUT)*1
            ),
            h.calls().at(0)
        );
    });
    sand_box([] ()
    {
        history h;
        atat::CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        atat::CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        atat::SendInput=
            [&] (UINT nInputs,LPINPUT pInputs,int cbSize)->UINT
            {
                h.calls().push_back(call
                (
                    "SendInput",
                    nInputs,
                    pInputs->type,
                    pInputs->ki.wScan,
                    pInputs->ki.dwFlags,
                    pInputs->ki.time,
                    pInputs->ki.dwExtraInfo,
                    cbSize
                ));
                return 1;
            };
        atat::GetDoubleClickTime=[&] ()->UINT {return 100;};
        atat::WaitForSingleObject=
            [&] (HANDLE hHandle,DWORD dwMilliseconds)->DWORD
            {return WAIT_TIMEOUT;};
        Context::instance()=make_shared<Context>();
        auto l=make_shared<Line>("key down B");
        auto kuc=make_shared<KeyUpCommand>(l);
        kuc->execute();
        CHECK_EQUAL(1,h.calls().size());
        CHECK_EQUAL
        (
            call
            (
                "SendInput",
                1,
                INPUT_KEYBOARD,
                0x30,
                KEYEVENTF_SCANCODE|KEYEVENTF_KEYUP,
                0,
                0,
                sizeof(INPUT)*1
            ),
            h.calls().at(0)
        );
    });
}

TEST_GROUP(KeyDownCommand) {};

TEST(KeyDownCommand,construct)
{
    sand_box([] ()
    {
        auto l=make_shared<Line>("key down A");
        auto kdc=make_shared<KeyDownCommand>(l);
        POINTERS_EQUAL(l.get(),kdc->line());
    });
}

TEST(KeyDownCommand,execute)
{
    sand_box([] ()
    {
        history h;
        atat::CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        atat::CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        atat::SendInput=
            [&] (UINT nInputs,LPINPUT pInputs,int cbSize)->UINT
            {
                h.calls().push_back(call
                (
                    "SendInput",
                    nInputs,
                    pInputs->type,
                    pInputs->ki.wScan,
                    pInputs->ki.dwFlags,
                    cbSize
                ));
                return 1;
            };
        atat::GetDoubleClickTime=
            [&] ()->UINT
            {
                h.calls().push_back(call("GetDoubleClickTime"));
                return 100;
            };
        atat::WaitForSingleObject=
            [&] (HANDLE hHandle,DWORD dwMilliseconds)->DWORD
            {
                h.calls().push_back(call
                (
                    "WaitForSingleObject",
                    hHandle,
                    dwMilliseconds
                ));
                return WAIT_TIMEOUT;
            };
        Context::instance()=make_shared<Context>();
        Context::instance()->index()=3;
        auto l=make_shared<Line>("key down A");
        auto kdc=make_shared<KeyDownCommand>(l);
        kdc->execute();
        CHECK_EQUAL(4,Context::instance()->index());
        CHECK_EQUAL(3,h.calls().size());
        CHECK_EQUAL
        (
            call
            (
                "SendInput",
                1,
                INPUT_KEYBOARD,
                0x1E,
                KEYEVENTF_SCANCODE,
                sizeof(INPUT)*1
            ),
            h.calls().at(0)
        );
        CHECK_EQUAL(call("GetDoubleClickTime"),h.calls().at(1));
        CHECK_EQUAL(call("WaitForSingleObject",(HANDLE)0x12,100),h.calls().at(2));
    });
}

TEST_GROUP(KeyPressCommand) {};

TEST(KeyPressCommand,construct)
{
    sand_box([] ()
    {
        auto l=make_shared<Line>("key press A");
        auto kpc=make_shared<KeyPressCommand>(l);
        POINTERS_EQUAL(l.get(),kpc->line());
    });
}

TEST(KeyPressCommand,execute)
{
    sand_box([] ()
    {
        history h;
        atat::CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        atat::CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        atat::SendInput=
            [&] (UINT nInputs,LPINPUT pInputs,int cbSize)->UINT
            {
                h.calls().push_back(call
                (
                    "SendInput",
                    nInputs,
                    pInputs->type,
                    pInputs->ki.wScan,
                    pInputs->ki.dwFlags,
                    cbSize
                ));
                return 1;
            };
        atat::GetDoubleClickTime=
            [&] ()->UINT
            {
                h.calls().push_back(call("GetDoubleClickTime"));
                return h.number_of("GetDoubleClickTime")*100;
            };
        atat::WaitForSingleObject=
            [&] (HANDLE hHandle,DWORD dwMilliseconds)->DWORD
            {
                h.calls().push_back(call
                (
                    "WaitForSingleObject",
                    hHandle,
                    dwMilliseconds
                ));
                return WAIT_TIMEOUT;
            };
        Context::instance()=make_shared<Context>();
        Context::instance()->index()=3;
        auto l=make_shared<Line>("key press A");
        auto kpc=make_shared<KeyPressCommand>(l);
        kpc->execute();
        CHECK_EQUAL(4,Context::instance()->index());
        CHECK_EQUAL(6,h.calls().size());
        CHECK_EQUAL
        (
            call
            (
                "SendInput",
                1,
                INPUT_KEYBOARD,
                0x1E,
                KEYEVENTF_SCANCODE,
                sizeof(INPUT)*1
            ),
            h.calls().at(0)
        );
        CHECK_EQUAL(call("GetDoubleClickTime"),h.calls().at(1));
        CHECK_EQUAL(call("WaitForSingleObject",(HANDLE)0x12,100),h.calls().at(2));
        CHECK_EQUAL
        (
            call
            (
                "SendInput",
                1,
                INPUT_KEYBOARD,
                0x1E,
                KEYEVENTF_SCANCODE|KEYEVENTF_KEYUP,
                sizeof(INPUT)*1
            ),
            h.calls().at(3)
        );
        CHECK_EQUAL(call("GetDoubleClickTime"),h.calls().at(4));
        CHECK_EQUAL(call("WaitForSingleObject",(HANDLE)0x12,200),h.calls().at(5));
    });
}

TEST_GROUP(KeyUpCommand) {};

TEST(KeyUpCommand,construct)
{
    sand_box([] ()
    {
        auto l=make_shared<Line>("key up A");
        auto kuc=make_shared<KeyUpCommand>(l);
        POINTERS_EQUAL(l.get(),kuc->line());
    });
}

TEST(KeyUpCommand,execute)
{
    sand_box([] ()
    {
        history h;
        atat::CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        atat::CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        atat::SendInput=
            [&] (UINT nInputs,LPINPUT pInputs,int cbSize)->UINT
            {
                h.calls().push_back(call
                (
                    "SendInput",
                    nInputs,
                    pInputs->type,
                    pInputs->ki.wScan,
                    pInputs->ki.dwFlags,
                    cbSize
                ));
                return 1;
            };
        atat::GetDoubleClickTime=
            [&] ()->UINT
            {
                h.calls().push_back(call("GetDoubleClickTime"));
                return 100;
            };
        atat::WaitForSingleObject=
            [&] (HANDLE hHandle,DWORD dwMilliseconds)->DWORD
            {
                h.calls().push_back(call
                (
                    "WaitForSingleObject",
                    hHandle,
                    dwMilliseconds
                ));
                return WAIT_TIMEOUT;
            };
        Context::instance()=make_shared<Context>();
        Context::instance()->index()=3;
        auto l=make_shared<Line>("key up A");
        auto kuc=make_shared<KeyUpCommand>(l);
        kuc->execute();
        CHECK_EQUAL(4,Context::instance()->index());
        CHECK_EQUAL(3,h.calls().size());
        CHECK_EQUAL
        (
            call
            (
                "SendInput",
                1,
                INPUT_KEYBOARD,
                0x1E,
                KEYEVENTF_SCANCODE|KEYEVENTF_KEYUP,
                sizeof(INPUT)*1
            ),
            h.calls().at(0)
        );
        CHECK_EQUAL(call("GetDoubleClickTime"),h.calls().at(1));
        CHECK_EQUAL(call("WaitForSingleObject",(HANDLE)0x12,100),h.calls().at(2));
    });
}

TEST_GROUP(Line) {};

TEST(Line,construct)
{
    sand_box([] ()
    {
        auto l=make_shared<Line>("abc def");
        CHECK_EQUAL("abc def",l->description());
        CHECK_EQUAL(2,l->tokens().size());
        CHECK_EQUAL("abc",l->tokens().at(0));
        CHECK_EQUAL("def",l->tokens().at(1));
    });
    sand_box([] ()
    {
        auto l=make_shared<Line>("abc#def");
        CHECK_EQUAL("abc#def",l->description());
        CHECK_EQUAL(1,l->tokens().size());
        CHECK_EQUAL("abc",l->tokens().at(0));
    });
}

TEST_GROUP(MouseButtonClickCommand) {};

TEST(MouseButtonClickCommand,construct)
{
    sand_box([] ()
    {
        auto l=make_shared<Line>("mouse left click");
        auto mbcc=make_shared<MouseButtonClickCommand>(l);
        POINTERS_EQUAL(l.get(),mbcc->line());
    });
}

TEST(MouseButtonClickCommand,execute)
{
    sand_box([] ()
    {
        history h;
        atat::CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        atat::CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        atat::SendInput=
            [&] (UINT nInputs,LPINPUT pInputs,int cbSize)->UINT
            {
                h.calls().push_back(call
                (
                    "SendInput",
                    nInputs,
                    pInputs->type,
                    pInputs->mi.dx,
                    pInputs->mi.dy,
                    pInputs->mi.mouseData,
                    pInputs->mi.dwFlags,
                    cbSize
                ));
                return 1;
            };
        atat::GetDoubleClickTime=
            [&] ()->UINT
            {
                h.calls().push_back(call("GetDoubleClickTime"));
                return h.number_of("GetDoubleClickTime")*100;
            };
        atat::WaitForSingleObject=
            [&] (HANDLE hHandle,DWORD dwMilliseconds)->DWORD
            {
                h.calls().push_back(call
                (
                    "WaitForSingleObject",
                    hHandle,
                    dwMilliseconds
                ));
                return WAIT_TIMEOUT;
            };
        Context::instance()=make_shared<Context>();
        Context::instance()->index()=3;
        auto l=make_shared<Line>("mouse left click");
        auto mbcc=make_shared<MouseButtonClickCommand>(l);
        mbcc->execute();
        CHECK_EQUAL(4,Context::instance()->index());
        CHECK_EQUAL(6,h.calls().size());
        CHECK_EQUAL
        (
            call
            (
                "SendInput",
                1,
                INPUT_MOUSE,
                0,
                0,
                0,
                MOUSEEVENTF_LEFTDOWN,
                sizeof(INPUT)*1
            ),
            h.calls().at(0)
        );
        CHECK_EQUAL(call("GetDoubleClickTime"),h.calls().at(1));
        CHECK_EQUAL(call("WaitForSingleObject",(HANDLE)0x12,100),h.calls().at(2));
        CHECK_EQUAL
        (
            call
            (
                "SendInput",
                1,
                INPUT_MOUSE,
                0,
                0,
                0,
                MOUSEEVENTF_LEFTUP,
                sizeof(INPUT)*1
            ),
            h.calls().at(3)
        );
        CHECK_EQUAL(call("GetDoubleClickTime"),h.calls().at(4));
        CHECK_EQUAL(call("WaitForSingleObject",(HANDLE)0x12,200),h.calls().at(5));
    });
}

TEST_GROUP(MouseButtonCommand) {};

TEST(MouseButtonCommand,construct)
{
    sand_box([] ()
    {
        auto l=make_shared<Line>("mouse left down down");
        try
        {
            auto mbdc=make_shared<MouseButtonDownCommand>(l);
            FAIL("Don't pass here.");
        } catch(const runtime_error&e)
        {
            STRCMP_EQUAL
            (
                "line:'mouse left down down':wrong number of tokens",
                e.what()
            );
        }
    });
    sand_box([] ()
    {
        auto l=make_shared<Line>("mouse left down");
        auto mbdc=make_shared<MouseButtonDownCommand>(l);
    });
    sand_box([] ()
    {
        auto l=make_shared<Line>("mouse right up");
        auto mbuc=make_shared<MouseButtonUpCommand>(l);
    });
}

TEST_GROUP(MouseButtonDoubleClickCommand) {};

TEST(MouseButtonDoubleClickCommand,construct)
{
    sand_box([] ()
    {
        auto l=make_shared<Line>("mouse left doubleclick");
        auto mbdcc=make_shared<MouseButtonDoubleClickCommand>(l);
        POINTERS_EQUAL(l.get(),mbdcc->line());
    });
}

TEST(MouseButtonDoubleClickCommand,execute)
{
    sand_box([] ()
    {
        history h;
        atat::CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        atat::CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        atat::SendInput=
            [&] (UINT nInputs,LPINPUT pInputs,int cbSize)->UINT
            {
                h.calls().push_back(call
                (
                    "SendInput",
                    nInputs,
                    pInputs->type,
                    pInputs->mi.dx,
                    pInputs->mi.dy,
                    pInputs->mi.mouseData,
                    pInputs->mi.dwFlags,
                    cbSize
                ));
                return 1;
            };
        atat::GetDoubleClickTime=
            [&] ()->UINT
            {
                h.calls().push_back(call("GetDoubleClickTime"));
                return h.number_of("GetDoubleClickTime")*100;
            };
        atat::WaitForSingleObject=
            [&] (HANDLE hHandle,DWORD dwMilliseconds)->DWORD
            {
                h.calls().push_back(call
                (
                    "WaitForSingleObject",
                    hHandle,
                    dwMilliseconds
                ));
                return WAIT_TIMEOUT;
            };
        Context::instance()=make_shared<Context>();
        Context::instance()->index()=3;
        auto l=make_shared<Line>("mouse left doubleclick");
        auto mbdcc=make_shared<MouseButtonDoubleClickCommand>(l);
        mbdcc->execute();
        CHECK_EQUAL(4,Context::instance()->index());
        CHECK_EQUAL(12,h.calls().size());
        CHECK_EQUAL
        (
            call
            (
                "SendInput",
                1,
                INPUT_MOUSE,
                0,
                0,
                0,
                MOUSEEVENTF_LEFTDOWN,
                sizeof(INPUT)*1
            ),
            h.calls().at(0)
        );
        CHECK_EQUAL(call("GetDoubleClickTime"),h.calls().at(1));
        CHECK_EQUAL(call("WaitForSingleObject",(HANDLE)0x12,25),h.calls().at(2));
        CHECK_EQUAL
        (
            call
            (
                "SendInput",
                1,
                INPUT_MOUSE,
                0,
                0,
                0,
                MOUSEEVENTF_LEFTUP,
                sizeof(INPUT)*1
            ),
            h.calls().at(3)
        );
        CHECK_EQUAL(call("GetDoubleClickTime"),h.calls().at(4));
        CHECK_EQUAL(call("WaitForSingleObject",(HANDLE)0x12,50),h.calls().at(5));
        CHECK_EQUAL
        (
            call
            (
                "SendInput",
                1,
                INPUT_MOUSE,
                0,
                0,
                0,
                MOUSEEVENTF_LEFTDOWN,
                sizeof(INPUT)*1
            ),
            h.calls().at(6)
        );
        CHECK_EQUAL(call("GetDoubleClickTime"),h.calls().at(7));
        CHECK_EQUAL(call("WaitForSingleObject",(HANDLE)0x12,75),h.calls().at(8));
        CHECK_EQUAL
        (
            call
            (
                "SendInput",
                1,
                INPUT_MOUSE,
                0,
                0,
                0,
                MOUSEEVENTF_LEFTUP,
                sizeof(INPUT)*1
            ),
            h.calls().at(9)
        );
        CHECK_EQUAL(call("GetDoubleClickTime"),h.calls().at(10));
        CHECK_EQUAL(call("WaitForSingleObject",(HANDLE)0x12,500),h.calls().at(11));
    });
}

TEST_GROUP(MouseButtonDownCommand) {};

TEST(MouseButtonDownCommand,construct)
{
    sand_box([] ()
    {
        auto l=make_shared<Line>("mouse left down");
        auto mbdc=make_shared<MouseButtonDownCommand>(l);
        POINTERS_EQUAL(l.get(),mbdc->line());
    });
}

TEST(MouseButtonDownCommand,execute)
{
    sand_box([] ()
    {
        history h;
        atat::CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        atat::CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        atat::SendInput=
            [&] (UINT nInputs,LPINPUT pInputs,int cbSize)->UINT
            {
                h.calls().push_back(call
                (
                    "SendInput",
                    nInputs,
                    pInputs->type,
                    pInputs->mi.dx,
                    pInputs->mi.dy,
                    pInputs->mi.mouseData,
                    pInputs->mi.dwFlags,
                    cbSize
                ));
                return 1;
            };
        atat::GetDoubleClickTime=
            [&] ()->UINT
            {
                h.calls().push_back(call("GetDoubleClickTime"));
                return 100;
            };
        atat::WaitForSingleObject=
            [&] (HANDLE hHandle,DWORD dwMilliseconds)->DWORD
            {
                h.calls().push_back(call
                (
                    "WaitForSingleObject",
                    hHandle,
                    dwMilliseconds
                ));
                return WAIT_TIMEOUT;
            };
        Context::instance()=make_shared<Context>();
        Context::instance()->index()=3;
        auto l=make_shared<Line>("mouse left down");
        auto mbdc=make_shared<MouseButtonDownCommand>(l);
        mbdc->execute();
        CHECK_EQUAL(4,Context::instance()->index());
        CHECK_EQUAL(3,h.calls().size());
        CHECK_EQUAL
        (
            call
            (
                "SendInput",
                1,
                INPUT_MOUSE,
                0,
                0,
                0,
                MOUSEEVENTF_LEFTDOWN,
                sizeof(INPUT)*1
            ),
            h.calls().at(0)
        );
        CHECK_EQUAL(call("GetDoubleClickTime"),h.calls().at(1));
        CHECK_EQUAL(call("WaitForSingleObject",(HANDLE)0x12,100),h.calls().at(2));
    });
}

TEST_GROUP(MouseButtonUpCommand) {};

TEST(MouseButtonUpCommand,construct)
{
    sand_box([] ()
    {
        auto l=make_shared<Line>("mouse left up");
        auto mbuc=make_shared<MouseButtonUpCommand>(l);
        POINTERS_EQUAL(l.get(),mbuc->line());
    });
}

TEST(MouseButtonUpCommand,execute)
{
    sand_box([] ()
    {
        history h;
        atat::CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        atat::CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        atat::SendInput=
            [&] (UINT nInputs,LPINPUT pInputs,int cbSize)->UINT
            {
                h.calls().push_back(call
                (
                    "SendInput",
                    nInputs,
                    pInputs->type,
                    pInputs->mi.dx,
                    pInputs->mi.dy,
                    pInputs->mi.mouseData,
                    pInputs->mi.dwFlags,
                    cbSize
                ));
                return 1;
            };
        atat::GetDoubleClickTime=
            [&] ()->UINT
            {
                h.calls().push_back(call("GetDoubleClickTime"));
                return 100;
            };
        atat::WaitForSingleObject=
            [&] (HANDLE hHandle,DWORD dwMilliseconds)->DWORD
            {
                h.calls().push_back(call
                (
                    "WaitForSingleObject",
                    hHandle,
                    dwMilliseconds
                ));
                return WAIT_TIMEOUT;
            };
        Context::instance()=make_shared<Context>();
        Context::instance()->index()=3;
        auto l=make_shared<Line>("mouse left up");
        auto mbuc=make_shared<MouseButtonUpCommand>(l);
        mbuc->execute();
        CHECK_EQUAL(4,Context::instance()->index());
        CHECK_EQUAL(3,h.calls().size());
        CHECK_EQUAL
        (
            call
            (
                "SendInput",
                1,
                INPUT_MOUSE,
                0,
                0,
                0,
                MOUSEEVENTF_LEFTUP,
                sizeof(INPUT)*1
            ),
            h.calls().at(0)
        );
        CHECK_EQUAL(call("GetDoubleClickTime"),h.calls().at(1));
        CHECK_EQUAL(call("WaitForSingleObject",(HANDLE)0x12,100),h.calls().at(2));
    });
}

TEST_GROUP(MouseCommand) {};

TEST(MouseCommand,send)
{
    sand_box([] ()
    {
        history h;
        atat::CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        atat::CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        atat::SendInput=
            [&] (UINT nInputs,LPINPUT pInputs,int cbSize)->UINT
            {
                h.calls().push_back(call
                (
                    "SendInput",
                    nInputs,
                    pInputs->type,
                    pInputs->mi.dx,
                    pInputs->mi.dy,
                    pInputs->mi.mouseData,
                    pInputs->mi.dwFlags,
                    pInputs->mi.time,
                    pInputs->mi.dwExtraInfo,
                    cbSize
                ));
                return 0;
            };
        atat::GetLastError=
            [&] ()->DWORD
            {
                h.calls().push_back(call("GetLastError"));
                return 34;
            };
        auto l=make_shared<Line>("mouse left down");
        auto mbdc=make_shared<MouseButtonDownCommand>(l);
        try
        {
            mbdc->execute();
            FAIL("Don't pass here.");
        } catch(const runtime_error&e)
        {STRCMP_EQUAL("function:'SendInput':failed(34)",e.what());}
        CHECK_EQUAL(2,h.calls().size());
        CHECK_EQUAL
        (
            call
            (
                "SendInput",
                1,
                INPUT_MOUSE,
                0,
                0,
                0,
                MOUSEEVENTF_LEFTDOWN,
                0,
                0,
                sizeof(INPUT)*1
            ),
            h.calls().at(0)
        );
        CHECK_EQUAL(call("GetLastError"),h.calls().at(1));
    });
    sand_box([] ()
    {
        history h;
        atat::CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        atat::CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        atat::SendInput=
            [&] (UINT nInputs,LPINPUT pInputs,int cbSize)->UINT
            {
                h.calls().push_back(call
                (
                    "SendInput",
                    nInputs,
                    pInputs->type,
                    pInputs->mi.dx,
                    pInputs->mi.dy,
                    pInputs->mi.mouseData,
                    pInputs->mi.dwFlags,
                    pInputs->mi.time,
                    pInputs->mi.dwExtraInfo,
                    cbSize
                ));
                return 1;
            };
        atat::GetDoubleClickTime=[&] ()->UINT {return 100;};
        atat::WaitForSingleObject=
            [&] (HANDLE hHandle,DWORD dwMilliseconds)->DWORD
            {return WAIT_TIMEOUT;};
        Context::instance()=make_shared<Context>();
        Context::instance()->index()=3;
        auto l=make_shared<Line>("mouse left down");
        auto mbdc=make_shared<MouseButtonDownCommand>(l);
        mbdc->execute();
        CHECK_EQUAL(4,Context::instance()->index());
        CHECK_EQUAL(1,h.calls().size());
        CHECK_EQUAL
        (
            call
            (
                "SendInput",
                1,
                INPUT_MOUSE,
                0,
                0,
                0,
                MOUSEEVENTF_LEFTDOWN,
                0,
                0,
                sizeof(INPUT)*1
            ),
            h.calls().at(0)
        );
    });
}

TEST_GROUP(MouseMoveCommand) {};

TEST(MouseMoveCommand,construct)
{
    sand_box([] ()
    {
        auto l=make_shared<Line>("mouse move");
        try
        {
            auto mmc=make_shared<MouseMoveCommand>(l);
            FAIL("Don't pass here.");
        } catch(const runtime_error&e)
        {
            STRCMP_EQUAL
            ("line:'mouse move':wrong number of tokens",e.what());
        }
    });
    sand_box([] ()
    {
        auto l=make_shared<Line>("mouse move 100");
        try
        {
            auto mmc=make_shared<MouseMoveCommand>(l);
            FAIL("Don't pass here.");
        } catch(const runtime_error&e)
        {
            STRCMP_EQUAL
            ("line:'mouse move 100':wrong number of tokens",e.what());
        }
    });
    sand_box([] ()
    {
        auto l=make_shared<Line>("mouse move 100 200 300");
        try
        {
            auto mmc=make_shared<MouseMoveCommand>(l);
            FAIL("Don't pass here.");
        } catch(const runtime_error&e)
        {
            STRCMP_EQUAL
            (
                "line:'mouse move 100 200 300':wrong number of tokens",
                e.what()
            );
        }
    });
    sand_box([] ()
    {
        auto l=make_shared<Line>("mouse move 100 200");
        auto mmc=make_shared<MouseMoveCommand>(l);
    });
}

TEST(MouseMoveCommand,execute)
{
    sand_box([] ()
    {
        history h;
        atat::CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        atat::CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        atat::GetSystemMetrics=
            [&] (int nIndex)->int
            {
                h.calls().push_back(call("GetSystemMetrics",nIndex));
                return 0;
            };
        Context::instance()=make_shared<Context>();
        auto l=make_shared<Line>("mouse move 100 200");
        auto mmc=make_shared<MouseMoveCommand>(l);
        try
        {
            mmc->execute();
            FAIL("Don't pass here");
        } catch(const runtime_error&e)
        {STRCMP_EQUAL("function:'GetSystemMetrics':failed",e.what());}
        CHECK_EQUAL(1,h.calls().size());
        CHECK_EQUAL(call("GetSystemMetrics",SM_CXSCREEN),h.calls().at(0));
    });
    sand_box([] ()
    {
        history h;
        atat::CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        atat::CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        atat::GetSystemMetrics=
            [&] (int nIndex)->int
            {
                h.calls().push_back(call("GetSystemMetrics",nIndex));
                int result;
                switch(h.number_of("GetSystemMetrics"))
                {
                case 1:
                    result=400;
                    break;
                case 2:
                    result=0;
                    break;
                default:
                    FAIL("Don't pass here");
                };
                return result;
            };
        Context::instance()=make_shared<Context>();
        auto l=make_shared<Line>("mouse move 100 200");
        auto mmc=make_shared<MouseMoveCommand>(l);
        try
        {
            mmc->execute();
            FAIL("Don't pass here");
        } catch(const runtime_error&e)
        {STRCMP_EQUAL("function:'GetSystemMetrics':failed",e.what());}
        CHECK_EQUAL(2,h.calls().size());
        CHECK_EQUAL(call("GetSystemMetrics",SM_CXSCREEN),h.calls().at(0));
        CHECK_EQUAL(call("GetSystemMetrics",SM_CYSCREEN),h.calls().at(1));
    });
    sand_box([] ()
    {
        history h;
        atat::CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        atat::CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        atat::GetSystemMetrics=
            [&] (int nIndex)->int
            {
                h.calls().push_back(call("GetSystemMetrics",nIndex));
                int result;
                switch(h.number_of("GetSystemMetrics"))
                {
                case 1:
                    result=400;
                    break;
                case 2:
                    result=300;
                    break;
                default:
                    FAIL("Don't pass here");
                };
                return result;
            };
        atat::SendInput=
            [&] (UINT nInputs,LPINPUT pInputs,int cbSize)->UINT
            {
                h.calls().push_back(call
                (
                    "SendInput",
                    nInputs,
                    pInputs->type,
                    pInputs->mi.dx,
                    pInputs->mi.dy,
                    pInputs->mi.mouseData,
                    pInputs->mi.dwFlags,
                    cbSize
                ));
                return 1;
            };
        Context::instance()=make_shared<Context>();
        Context::instance()->index()=3;
        auto l=make_shared<Line>("mouse move 100 200");
        auto mmc=make_shared<MouseMoveCommand>(l);
        mmc->execute();
        CHECK_EQUAL(4,Context::instance()->index());
        CHECK_EQUAL(3,h.calls().size());
        CHECK_EQUAL(call("GetSystemMetrics",SM_CXSCREEN),h.calls().at(0));
        CHECK_EQUAL(call("GetSystemMetrics",SM_CYSCREEN),h.calls().at(1));
        CHECK_EQUAL
        (
            call
            (
                "SendInput",
                1,
                INPUT_MOUSE,
                (100<<16)/400,
                (200<<16)/300,
                0,
                MOUSEEVENTF_MOVE|MOUSEEVENTF_ABSOLUTE,
                sizeof(INPUT)*1
            ),
            h.calls().at(2)
        );
    });
    sand_box([] ()
    {
        history h;
        atat::CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        atat::CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        atat::FindWindowW=
            [&]
            (const wchar_t*lpClassName,const wchar_t*lpWindowName)->HWND
            {return (HWND)0x12;};
        atat::GetWindowRect=
            [&] (HWND hWnd,LPRECT lpRect)->BOOL
            {
                h.calls().push_back(call("GetWindowRect",hWnd));
                return FALSE;
            };
        atat::GetLastError=
            [&] ()->DWORD
            {
                h.calls().push_back(call("GetLastError"));
                return 34;
            };
        Context::instance()=make_shared<Context>();
        Context::instance()->properties().insert({"target","電卓"});
        auto l=make_shared<Line>("mouse move 100 200");
        auto mmc=make_shared<MouseMoveCommand>(l);
        try
        {
            mmc->execute();
            FAIL("Don't pass here.");
        } catch(const runtime_error&e)
        {STRCMP_EQUAL("function:'GetWindowRect':failed(34)",e.what());}
        CHECK_EQUAL(2,h.calls().size());
        CHECK_EQUAL(call("GetWindowRect",(HWND)0x12),h.calls().at(0));
        CHECK_EQUAL(call("GetLastError"),h.calls().at(1));
    });
    sand_box([] ()
    {
        history h;
        atat::CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        atat::CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        atat::FindWindowW=
            [&]
            (const wchar_t*lpClassName,const wchar_t*lpWindowName)->HWND
            {
                h.calls().push_back(call
                (
                    "FindWindowW",
                    (const char*)lpClassName,
                    (const char*)lpWindowName
                ));
                return (HWND)0x56;
            };
        atat::GetWindowRect=
            [&] (HWND hWnd,LPRECT lpRect)->BOOL
            {
                h.calls().push_back(call("GetWindowRect",hWnd));
                lpRect->left=30;
                lpRect->top=60;
                return TRUE;
            };
        atat::GetSystemMetrics=
            [&] (int nIndex)->int
            {
                h.calls().push_back(call("GetSystemMetrics",nIndex));
                int result;
                switch(h.number_of("GetSystemMetrics"))
                {
                case 1:
                    result=400;
                    break;
                case 2:
                    result=300;
                    break;
                default:
                    FAIL("Don't pass here");
                };
                return result;
            };
        atat::SendInput=
            [&] (UINT nInputs,LPINPUT pInputs,int cbSize)->UINT
            {
                h.calls().push_back(call
                (
                    "SendInput",
                    nInputs,
                    pInputs->type,
                    pInputs->mi.dx,
                    pInputs->mi.dy,
                    pInputs->mi.mouseData,
                    pInputs->mi.dwFlags,
                    cbSize
                ));
                return 1;
            };
        Context::instance()=make_shared<Context>();
        properties().insert({"target","電卓"});
        Context::instance()->index()=3;
        auto l=make_shared<Line>("mouse move 100 200");
        auto mmc=make_shared<MouseMoveCommand>(l);
        mmc->execute();
        CHECK_EQUAL(4,Context::instance()->index());
        CHECK_EQUAL(5,h.calls().size());
        CHECK_EQUAL
        (
            call
            (
                "FindWindowW",
                (const char*)NULL,
                (const char*)L"電卓"
            ),
            h.calls().at(0)
        );
        CHECK_EQUAL(call("GetWindowRect",(HWND)0x56),h.calls().at(1));
        CHECK_EQUAL(call("GetSystemMetrics",SM_CXSCREEN),h.calls().at(2));
        CHECK_EQUAL(call("GetSystemMetrics",SM_CYSCREEN),h.calls().at(3));
        CHECK_EQUAL
        (
            call
            (
                "SendInput",
                1,
                INPUT_MOUSE,
                (130<<16)/400,
                (260<<16)/300,
                0,
                MOUSEEVENTF_MOVE|MOUSEEVENTF_ABSOLUTE,
                sizeof(INPUT)*1
            ),
            h.calls().at(4)
        );
    });
}

TEST_GROUP(MouseWheelCommand) {};

TEST(MouseWheelCommand,construct)
{
    sand_box([] ()
    {
        auto l=make_shared<Line>("mouse wheel");
        try
        {
            auto mwc=make_shared<MouseWheelCommand>(l);
            FAIL("Don't pass here.");
        } catch(const runtime_error&e)
        {
            STRCMP_EQUAL
            ("line:'mouse wheel':wrong number of tokens",e.what());
        }
    });
    sand_box([] ()
    {
        auto l=make_shared<Line>("mouse wheel 10 10");
        try
        {
            auto mwc=make_shared<MouseWheelCommand>(l);
            FAIL("Don't pass here.");
        } catch(const runtime_error&e)
        {
            STRCMP_EQUAL
            ("line:'mouse wheel 10 10':wrong number of tokens",e.what());
        }
    });
    sand_box([] ()
    {
        auto l=make_shared<Line>("mouse wheel 10");
        auto mwc=make_shared<MouseWheelCommand>(l);
    });
}

TEST(MouseWheelCommand,execute)
{
    sand_box([] ()
    {
        history h;
        atat::CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        atat::CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        atat::SendInput=
            [&] (UINT nInputs,LPINPUT pInputs,int cbSize)->UINT
            {
                h.calls().push_back(call
                (
                    "SendInput",
                    nInputs,
                    pInputs->type,
                    pInputs->mi.dx,
                    pInputs->mi.dy,
                    (LONG)pInputs->mi.mouseData,
                    pInputs->mi.dwFlags,
                    cbSize
                ));
                return 1;
            };
        Context::instance()=make_shared<Context>();
        Context::instance()->index()=3;
        auto l=make_shared<Line>("mouse wheel 10");
        auto mwc=make_shared<MouseWheelCommand>(l);
        mwc->execute();
        CHECK_EQUAL(4,Context::instance()->index());
        CHECK_EQUAL(1,h.calls().size());
        CHECK_EQUAL
        (
            call
            (
                "SendInput",
                1,
                INPUT_MOUSE,
                0,
                0,
                10,
                MOUSEEVENTF_WHEEL,
                sizeof(INPUT)*1
            ),
            h.calls().at(0)
        );
    });
    sand_box([] ()
    {
        history h;
        atat::CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        atat::CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        atat::SendInput=
            [&] (UINT nInputs,LPINPUT pInputs,int cbSize)->UINT
            {
                h.calls().push_back(call
                (
                    "SendInput",
                    nInputs,
                    pInputs->type,
                    pInputs->mi.dx,
                    pInputs->mi.dy,
                    (LONG)pInputs->mi.mouseData,
                    pInputs->mi.dwFlags,
                    cbSize
                ));
                return 1;
            };
        Context::instance()=make_shared<Context>();
        Context::instance()->index()=3;
        auto l=make_shared<Line>("mouse wheel -10");
        auto mwc=make_shared<MouseWheelCommand>(l);
        mwc->execute();
        CHECK_EQUAL(4,Context::instance()->index());
        CHECK_EQUAL(1,h.calls().size());
        CHECK_EQUAL
        (
            call
            (
                "SendInput",
                1,
                INPUT_MOUSE,
                0,
                0,
                -10,
                MOUSEEVENTF_WHEEL,
                sizeof(INPUT)*1
            ),
            h.calls().at(0)
        );
    });
}

TEST_GROUP(LoopBeginCommand) {};

TEST(LoopBeginCommand,construct)
{
    sand_box([] ()
    {
        auto l=make_shared<Line>("loop begin 5 5");
        try
        {
            auto lbc=make_shared<LoopBeginCommand>(l);
            FAIL("Don't pass here.");
        } catch(const runtime_error&e)
        {
            STRCMP_EQUAL
            ("line:'loop begin 5 5':wrong number of tokens",e.what());
        }
    });
    sand_box([] ()
    {
        auto l=make_shared<Line>("loop begin");
        auto lbc=make_shared<LoopBeginCommand>(l);
    });
    sand_box([] ()
    {
        auto l=make_shared<Line>("loop begin 5");
        auto lbc=make_shared<LoopBeginCommand>(l);
    });
}

TEST(LoopBeginCommand,execute)
{
    sand_box([] ()
    {
        atat::CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        atat::CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        Context::instance()=make_shared<Context>();
        Context::instance()->index()=3;
        auto l=make_shared<Line>("loop begin");
        auto lbc=make_shared<LoopBeginCommand>(l);
        lbc->execute();
        CHECK_EQUAL(4,Context::instance()->index());
        CHECK_EQUAL(2,Context::instance()->frames().size());
        CHECK_EQUAL(0,Context::instance()->frames().back().counter);
        CHECK_EQUAL(4,Context::instance()->frames().back().entry);
        CHECK_EQUAL(0,Context::instance()->frames().back().number);
    });
    sand_box([] ()
    {
        atat::CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        atat::CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        Context::instance()=make_shared<Context>();
        Context::instance()->index()=3;
        auto l=make_shared<Line>("loop begin 5");
        auto lbc=make_shared<LoopBeginCommand>(l);
        lbc->execute();
        CHECK_EQUAL(4,Context::instance()->index());
        CHECK_EQUAL(2,Context::instance()->frames().size());
        CHECK_EQUAL(0,Context::instance()->frames().back().counter);
        CHECK_EQUAL(4,Context::instance()->frames().back().entry);
        CHECK_EQUAL(5,Context::instance()->frames().back().number);
    });
}

TEST_GROUP(LoopEndCommand) {};

TEST(LoopEndCommand,construct)
{
    sand_box([] ()
    {
        auto l=make_shared<Line>("loop end end");
        try
        {
            auto lec=make_shared<LoopEndCommand>(l);
            FAIL("Don't pass here.");
        } catch(const runtime_error&e)
        {
            STRCMP_EQUAL
            ("line:'loop end end':wrong number of tokens",e.what());
        }
    });
    sand_box([] ()
    {
        auto l=make_shared<Line>("loop end");
        auto lec=make_shared<LoopEndCommand>(l);
    });
}

TEST(LoopEndCommand,execute)
{
    sand_box([] ()
    {
        atat::CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        atat::CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        Context::instance()=make_shared<Context>();
        Context::instance()->index()=6;
        Context::instance()->frames().push_back({0,4,0});
        auto l=make_shared<Line>("loop end");
        auto lec=make_shared<LoopEndCommand>(l);
        lec->execute();
        CHECK_EQUAL(4,Context::instance()->index());
        CHECK_EQUAL(2,Context::instance()->frames().size());
        CHECK_EQUAL(0,Context::instance()->frames().back().counter);
        CHECK_EQUAL(4,Context::instance()->frames().back().entry);
        CHECK_EQUAL(0,Context::instance()->frames().back().number);
    });
    sand_box([] ()
    {
        atat::CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        atat::CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        Context::instance()=make_shared<Context>();
        Context::instance()->index()=6;
        Context::instance()->frames().push_back({3,4,5});
        auto l=make_shared<Line>("loop end");
        auto lec=make_shared<LoopEndCommand>(l);
        lec->execute();
        CHECK_EQUAL(4,Context::instance()->index());
        CHECK_EQUAL(2,Context::instance()->frames().size());
        CHECK_EQUAL(4,Context::instance()->frames().back().counter);
        CHECK_EQUAL(4,Context::instance()->frames().back().entry);
        CHECK_EQUAL(5,Context::instance()->frames().back().number);
    });
    sand_box([] ()
    {
        atat::CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        atat::CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        Context::instance()=make_shared<Context>();
        Context::instance()->index()=6;
        Context::instance()->frames().push_back({4,4,5});
        auto l=make_shared<Line>("loop end");
        auto lec=make_shared<LoopEndCommand>(l);
        lec->execute();
        CHECK_EQUAL(7,Context::instance()->index());
        CHECK_EQUAL(1,Context::instance()->frames().size());
    });
}

TEST_GROUP(SleepCommand) {};

TEST(SleepCommand,construct)
{
    sand_box([] ()
    {
        auto l=make_shared<Line>("sleep");
        try
        {
            auto sc=make_shared<SleepCommand>(l);
            FAIL("Don't pass here.");
        } catch(const runtime_error&e)
        {
            STRCMP_EQUAL
            ("line:'sleep':wrong number of tokens",e.what());
        }
    });
    sand_box([] ()
    {
        auto l=make_shared<Line>("sleep 3000 3000");
        try
        {
            auto sc=make_shared<SleepCommand>(l);
            FAIL("Don't pass here.");
        } catch(const runtime_error&e)
        {
            STRCMP_EQUAL
            ("line:'sleep 3000 3000':wrong number of tokens",e.what());
        }
    });
    sand_box([] ()
    {
        auto l=make_shared<Line>("sleep 3000");
        auto sc=make_shared<SleepCommand>(l);
    });
}

TEST(SleepCommand,execute)
{
    sand_box([] ()
    {
        history h;
        atat::CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        atat::CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        atat::WaitForSingleObject=
            [&] (HANDLE hHandle,DWORD dwMilliseconds)->DWORD
            {
                h.calls().push_back(call
                (
                    "WaitForSingleObject",
                    hHandle,
                    dwMilliseconds
                ));
                return WAIT_TIMEOUT;
            };
        Context::instance()=make_shared<Context>();
        Context::instance()->index()=3;
        auto l=make_shared<Line>("sleep 3000");
        auto sc=make_shared<SleepCommand>(l);
        sc->execute();
        CHECK_EQUAL(4,Context::instance()->index());
        CHECK_EQUAL(1,h.calls().size());
        CHECK_EQUAL
        (call("WaitForSingleObject",(HANDLE)0x12,3000),h.calls().at(0));
    });
}

TEST_GROUP(SystemObject) {};

TEST(SystemObject,construct)
{
    sand_box([] ()
    {
        atat::CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        auto so=make_shared<SystemObject>((HANDLE)NULL);
        CHECK_EQUAL((HANDLE)NULL,so->handle());
    });
    sand_box([] ()
    {
        atat::CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        auto so=make_shared<SystemObject>((HANDLE)0x12);
        CHECK_EQUAL((HANDLE)0x12,so->handle());
    });
}

TEST(SystemObject,destruct)
{
    sand_box([] ()
    {
        atat::CloseHandle=
            [&] (HANDLE hObject)->BOOL
            {
                FAIL("Don't pass here.");
                return TRUE;
            };
        make_shared<SystemObject>((HANDLE)NULL);
    });
    sand_box([] ()
    {
        history h;
        atat::CloseHandle=
            [&] (HANDLE hObject)->BOOL
            {
                h.calls().push_back(call("CloseHandle",hObject));
                return TRUE;
            };
        make_shared<SystemObject>((HANDLE)0x12);
        CHECK_EQUAL(1,h.calls().size());
        CHECK_EQUAL(call("CloseHandle",(HANDLE)0x12),h.calls().at(0));
    });
}

TEST_GROUP(free) {};

TEST(free,control_key_pressed)
{
    sand_box([] ()
    {
        BOOL handled=control_key_pressed(CTRL_BREAK_EVENT);
        CHECK_EQUAL(FALSE,handled);
    });
    sand_box([] ()
    {
        history h;
        atat::CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        atat::CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        atat::SetEvent=
            [&] (HANDLE hEvent)->BOOL
            {
                h.calls().push_back(call("SetEvent",hEvent));
                return TRUE;
            };
        Context::instance()=make_shared<Context>();
        BOOL handled=control_key_pressed(CTRL_C_EVENT);
        CHECK_EQUAL(TRUE,handled);
        CHECK_EQUAL(1,h.calls().size());
        CHECK_EQUAL(call("SetEvent",(HANDLE)0x12),h.calls().at(0));
    });
}

TEST(free,describe)
{
    sand_box([] ()
    {
        CHECK_EQUAL("abc",describe("abc"));
        CHECK_EQUAL("123def",describe(123,"def"));
    });
}

TEST(free,describe_with)
{
    sand_box([] ()
    {
        CHECK_EQUAL("abc",describe_with(",","abc"));
        CHECK_EQUAL("123,def",describe_with(",",123,"def"));
    });
}

TEST(free,find_target)
{
    sand_box([] ()
    {
        atat::CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        atat::CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        Context::instance()=make_shared<Context>();
        try
        {
            find_target();
            FAILED("Don't pass here.");
        } catch(const runtime_error&e)
        {STRCMP_EQUAL("property:'target':not found",e.what());}
    });
    sand_box([] ()
    {
        history h;
        atat::CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        atat::CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        atat::FindWindowW=
            [&]
            (const wchar_t*lpClassName,const wchar_t*lpWindowName)->HWND
            {
                h.calls().push_back(call
                (
                    "FindWindowW",
                    (const char*)lpClassName,
                    (const char*)lpWindowName
                ));
                return NULL;
            };
        Context::instance()=make_shared<Context>();
        properties().insert({"target","電卓"});
        try
        {
            find_target();
            FAIL("Don't pass here.");
        } catch(const runtime_error&e)
        {STRCMP_EQUAL("target:'電卓':not found",e.what());}
        CHECK_EQUAL(1,h.calls().size());
        CHECK_EQUAL
        (
            call
            (
                "FindWindowW",
                (const char*)NULL,
                (const char*)L"電卓"
            ),
            h.calls().at(0)
        );
    });
    sand_box([] ()
    {
        history h;
        atat::CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        atat::CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        atat::FindWindowW=
            [&]
            (const wchar_t*lpClassName,const wchar_t*lpWindowName)->HWND
            {
                h.calls().push_back(call
                (
                    "FindWindowW",
                    (const char*)lpClassName,
                    (const char*)lpWindowName
                ));
                return (HWND)0x56;
            };
        Context::instance()=make_shared<Context>();
        Context::instance()->properties().insert({"target","電卓"});
        HWND w=find_target();
        CHECK_EQUAL((HWND)0x56,w);
        CHECK_EQUAL(1,h.calls().size());
        CHECK_EQUAL
        (
            call
            (
                "FindWindowW",
                (const char*)NULL,
                (const char*)L"電卓"
            ),
            h.calls().at(0)
        );
    });
}

TEST(free,lower_case)
{
    sand_box([] ()
    {
        CHECK_EQUAL("abc",lower_case("ABC"));
        CHECK_EQUAL("def",lower_case("def"));
        CHECK_EQUAL("123",lower_case("123"));
    });
}

TEST(free,multi_to_wide)
{
    sand_box([] ()
    {
        MEMCMP_EQUAL(L"いろは",multi_to_wide("いろは",CP_UTF8).get(),8);
        MEMCMP_EQUAL(L"にほへと",multi_to_wide("にほへと",CP_UTF8).get(),10);
    });
}

TEST(free,new_command)
{
    sand_box([] ()
    {
        map<string,COMMAND_FACTORY> fs(
        {
            {
                "down",
                [] (const shared_ptr<Line>&line)->shared_ptr<Command>
                {return make_shared<KeyDownCommand>(line);}
            },
            {
                "up",
                [] (const shared_ptr<Line>&line)->shared_ptr<Command>
                {return make_shared<KeyUpCommand>(line);}
            },
        });
        auto l=make_shared<Line>("");
        CHECK(dynamic_cast<NullCommand*>(new_command(fs,l,0).get()));
    });
    sand_box([] ()
    {
        map<string,COMMAND_FACTORY> fs(
        {
            {
                "down",
                [] (const shared_ptr<Line>&line)->shared_ptr<Command>
                {return make_shared<KeyDownCommand>(line);}
            },
            {
                "up",
                [] (const shared_ptr<Line>&line)->shared_ptr<Command>
                {return make_shared<KeyUpCommand>(line);}
            },
        });
        auto l=make_shared<Line>("key down A");
        try
        {
            auto c=new_command(fs,l,3);
            FAIL("Don't pass here.");
        } catch(const runtime_error&e)
        {STRCMP_EQUAL("line:'key down A':few switches",e.what());}
    });
    sand_box([] ()
    {
        map<string,COMMAND_FACTORY> fs(
        {
            {
                "down",
                [] (const shared_ptr<Line>&line)->shared_ptr<Command>
                {return make_shared<KeyDownCommand>(line);}
            },
            {
                "up",
                [] (const shared_ptr<Line>&line)->shared_ptr<Command>
                {return make_shared<KeyUpCommand>(line);}
            },
        });
        auto l=make_shared<Line>("key press A");
        try
        {
            auto c=new_command(fs,l,1);
            FAIL("Don't pass here.");
        } catch(const runtime_error&e)
        {STRCMP_EQUAL("switch:'press':unknown",e.what());}
    });
    sand_box([] ()
    {
        map<string,COMMAND_FACTORY> fs(
        {
            {
                "down",
                [] (const shared_ptr<Line>&line)->shared_ptr<Command>
                {return make_shared<KeyDownCommand>(line);}
            },
            {
                "up",
                [] (const shared_ptr<Line>&line)->shared_ptr<Command>
                {return make_shared<KeyUpCommand>(line);}
            },
        });
        auto l=make_shared<Line>("key down A");
        CHECK(dynamic_cast<KeyDownCommand*>(new_command(fs,l,1).get()));
    });
    sand_box([] ()
    {
        map<string,COMMAND_FACTORY> fs(
        {
            {
                "down",
                [] (const shared_ptr<Line>&line)->shared_ptr<Command>
                {return make_shared<KeyDownCommand>(line);}
            },
            {
                "up",
                [] (const shared_ptr<Line>&line)->shared_ptr<Command>
                {return make_shared<KeyUpCommand>(line);}
            },
        });
        auto l=make_shared<Line>("key DoWn A");
        CHECK(dynamic_cast<KeyDownCommand*>(new_command(fs,l,1).get()));
    });
    sand_box([] ()
    {
        map<string,COMMAND_FACTORY> fs(
        {
            {
                "down",
                [] (const shared_ptr<Line>&line)->shared_ptr<Command>
                {return make_shared<KeyDownCommand>(line);}
            },
            {
                "up",
                [] (const shared_ptr<Line>&line)->shared_ptr<Command>
                {return make_shared<KeyUpCommand>(line);}
            },
        });
        auto l=make_shared<Line>("key up A");
        CHECK(dynamic_cast<KeyUpCommand*>(new_command(fs,l,1).get()));
    });
}

TEST(free,parse_list)
{
    sand_box([] ()
    {
        atat::CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        atat::CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        istringstream iss
        (
            "key down A\n"
            "key up A\n"
            "key press A\n"
            "mouse left down\n"
            "mouse left up\n"
            "mouse left click\n"
            "mouse left doubleclick\n"
            "mouse right down\n"
            "mouse right up\n"
            "mouse right click\n"
            "mouse right doubleclick\n"
            "mouse middle down\n"
            "mouse middle up\n"
            "mouse middle click\n"
            "mouse middle doubleclick\n"
            "loop begin 5\n"
            "loop end\n"
            "sleep 3000\n"
            " \n"
        );
        Context::instance()=make_shared<Context>();
        auto cs=parse_list(iss);
        CHECK_EQUAL(19,cs.size());
        CHECK(dynamic_cast<KeyDownCommand*>(cs.at(0).get()));
        CHECK(dynamic_cast<KeyUpCommand*>(cs.at(1).get()));
        CHECK(dynamic_cast<KeyPressCommand*>(cs.at(2).get()));
        CHECK(dynamic_cast<MouseButtonDownCommand*>(cs.at(3).get()));
        CHECK(dynamic_cast<MouseButtonUpCommand*>(cs.at(4).get()));
        CHECK(dynamic_cast<MouseButtonClickCommand*>(cs.at(5).get()));
        CHECK(dynamic_cast<MouseButtonDoubleClickCommand*>(cs.at(6).get()));
        CHECK(dynamic_cast<MouseButtonDownCommand*>(cs.at(7).get()));
        CHECK(dynamic_cast<MouseButtonUpCommand*>(cs.at(8).get()));
        CHECK(dynamic_cast<MouseButtonClickCommand*>(cs.at(9).get()));
        CHECK(dynamic_cast<MouseButtonDoubleClickCommand*>(cs.at(10).get()));
        CHECK(dynamic_cast<MouseButtonDownCommand*>(cs.at(11).get()));
        CHECK(dynamic_cast<MouseButtonUpCommand*>(cs.at(12).get()));
        CHECK(dynamic_cast<MouseButtonClickCommand*>(cs.at(13).get()));
        CHECK(dynamic_cast<MouseButtonDoubleClickCommand*>(cs.at(14).get()));
        CHECK(dynamic_cast<LoopBeginCommand*>(cs.at(15).get()));
        CHECK(dynamic_cast<LoopEndCommand*>(cs.at(16).get()));
        CHECK(dynamic_cast<SleepCommand*>(cs.at(17).get()));
        CHECK(dynamic_cast<NullCommand*>(cs.at(18).get()));
        CHECK_EQUAL(19,Context::instance()->index());
    });
}

TEST(free,parse_properties)
{
    sand_box([] ()
    {
        static const char*argv[]=
        {
            "test",
            "a",
            "b=c",
        };
        auto ps=parse_properties(3,(char**)argv);
        CHECK_EQUAL(2,ps.size());
        CHECK(ps.find("a")!=ps.end());
        CHECK_EQUAL("",ps.at("a"));
        CHECK(ps.find("b")!=ps.end());
        CHECK_EQUAL("c",ps.at("b"));
        CHECK(ps.find("d")==ps.end());
    });
}

TEST(free,properties)
{
    sand_box([] ()
    {
        atat::CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        atat::CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        Context::instance()=make_shared<Context>();
        POINTERS_EQUAL(&Context::instance()->properties(),&properties());
    });
}

TEST(free,run)
{
    sand_box([] ()
    {
        atat::CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        atat::CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        Context::instance()=make_shared<Context>();
        static const char*argv[]={"test","help"};
        istringstream in("");
        ostringstream out,err;
        int result=atat::run(2,(char**)argv,in,out,err);
        CHECK_EQUAL(1,result);
        CHECK_EQUAL("",out.str());
        CHECK_EQUAL("usage",err.str().substr(0,5));
    });
    sand_box([] ()
    {
        history h;
        atat::CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        atat::CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        atat::SetConsoleCtrlHandler=
            [&] (PHANDLER_ROUTINE HandlerRoutine,BOOL Add)->BOOL
            {
                h.calls().push_back(call
                (
                    "SetConsoleCtrlHandler",
                    HandlerRoutine,
                    Add
                ));
                return FALSE;
            };
        atat::GetLastError=
            [&] ()->DWORD
            {
                h.calls().push_back(call("GetLastError"));
                return 34;
            };
        Context::instance()=make_shared<Context>();
        static const char*argv[]={"test"};
        istringstream in("");
        ostringstream out,err;
        int result=atat::run(2,(char**)argv,in,out,err);
        CHECK_EQUAL(1,result);
        CHECK_EQUAL("",out.str());
        CHECK_EQUAL
        (
            "error(0):function:'SetConsoleCtrlHandler':failed(34)\n",
            err.str()
        );
        CHECK_EQUAL(2,h.calls().size());
        CHECK_EQUAL
        (
            call
            (
                "SetConsoleCtrlHandler",
                (PHANDLER_ROUTINE)control_key_pressed,
                TRUE
            ),
            h.calls().at(0)
        );
        CHECK_EQUAL(call("GetLastError"),h.calls().at(1));
    });
    sand_box([] ()
    {
        history h;
        atat::CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        atat::CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        atat::SetConsoleCtrlHandler=
            [&] (PHANDLER_ROUTINE HandlerRoutine,BOOL Add)->BOOL
            {return TRUE;};
        atat::WaitForSingleObject=
            [&] (HANDLE hHandle,DWORD dwMilliseconds)->DWORD
            {
                h.calls().push_back(call
                (
                    "WaitForSingleObject",
                    hHandle,
                    dwMilliseconds
                ));
                DWORD result;
                switch(h.number_of("WaitForSingleObject"))
                {
                case 1:
                    result=WAIT_TIMEOUT;
                    break;
                case 2:
                    result=WAIT_OBJECT_0;
                    break;
                default:
                    FAIL("Don't pass here");
                };
                return result;
            };
        Context::instance()=make_shared<Context>();
        static const char*argv[]={"test"};
        istringstream in
        (
            "sleep 100\n"
            "sleep 100\n"
        );
        ostringstream out,err;
        int result=atat::run(1,(char**)argv,in,out,err);
        CHECK_EQUAL(0,result);
        CHECK_EQUAL
        (
            "sleep 100\n"
            "sleep 100\n",
            out.str()
        );
        CHECK_EQUAL("",err.str());
        CHECK_EQUAL(1,Context::instance()->index());
        CHECK_EQUAL(2,h.calls().size());
        CHECK_EQUAL
        (call("WaitForSingleObject",(HANDLE)0x12,100),h.calls().at(0));
        CHECK_EQUAL
        (call("WaitForSingleObject",(HANDLE)0x12,100),h.calls().at(1));
    });
    sand_box([] ()
    {
        history h;
        atat::CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        atat::CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        atat::FindWindowW=
            [&]
            (const wchar_t*lpClassName,const wchar_t*lpWindowName)->HWND
            {
                h.calls().push_back(call
                (
                    "FindWindowW",
                    (const char*)lpClassName,
                    (const char*)lpWindowName
                ));
                return (HWND)0x56;
            };
        atat::GetForegroundWindow=[&] ()->HWND {return (HWND)0x56;};
        atat::SetConsoleCtrlHandler=
            [&] (PHANDLER_ROUTINE HandlerRoutine,BOOL Add)->BOOL
            {
                h.calls().push_back(call
                (
                    "SetConsoleCtrlHandler",
                    HandlerRoutine,
                    Add
                ));
                return TRUE;
            };
        atat::WaitForSingleObject=
            [&] (HANDLE hHandle,DWORD dwMilliseconds)->DWORD
            {
                h.calls().push_back(call
                (
                    "WaitForSingleObject",
                    hHandle,
                    dwMilliseconds
                ));
                return WAIT_TIMEOUT;
            };
        Context::instance()=make_shared<Context>();
        static const char*argv[]={"test","target=GAME"};
        istringstream in
        (
            "loop begin 2\n"
            "sleep 100\n"
            "loop end\n"
        );
        ostringstream out,err;
        int result=atat::run(2,(char**)argv,in,out,err);
        CHECK_EQUAL(0,result);
        CHECK_EQUAL
        (
            "loop begin 2\n"
            "sleep 100\n"
            "loop end\n"
            "sleep 100\n"
            "loop end\n",
            out.str()
        );
        CHECK_EQUAL("",err.str());
        CHECK_EQUAL(3,Context::instance()->index());
        CHECK_EQUAL(8,h.calls().size());
        CHECK_EQUAL
        (
            call
            (
                "SetConsoleCtrlHandler",
                (PHANDLER_ROUTINE)control_key_pressed,
                TRUE
            ),
            h.calls().at(0)
        );
        CHECK_EQUAL
        (
            call
            (
                "FindWindowW",
                (const char*)NULL,
                (const char*)L"電卓"
            ),
            h.calls().at(1)
        );
        CHECK_EQUAL
        (
            call
            (
                "FindWindowW",
                (const char*)NULL,
                (const char*)L"電卓"
            ),
            h.calls().at(2)
        );
        CHECK_EQUAL
        (call("WaitForSingleObject",(HANDLE)0x12,100),h.calls().at(3));
        CHECK_EQUAL
        (
            call
            (
                "FindWindowW",
                (const char*)NULL,
                (const char*)L"電卓"
            ),
            h.calls().at(4)
        );
        CHECK_EQUAL
        (
            call
            (
                "FindWindowW",
                (const char*)NULL,
                (const char*)L"電卓"
            ),
            h.calls().at(5)
        );
        CHECK_EQUAL
        (call("WaitForSingleObject",(HANDLE)0x12,100),h.calls().at(6));
        CHECK_EQUAL
        (
            call
            (
                "FindWindowW",
                (const char*)NULL,
                (const char*)L"電卓"
            ),
            h.calls().at(7)
        );
    });
}

TEST(free,setup_io)
{
    sand_box([] ()
    {
        setup_io();
        POINTERS_EQUAL
        (
            ::CloseHandle,
            *atat::CloseHandle.target<BOOL(*)(HANDLE)>()
        );
        POINTERS_EQUAL
        (
            ::CreateEvent,
            *atat::CreateEvent.target
            <HANDLE(*)(LPSECURITY_ATTRIBUTES,BOOL,BOOL,LPCTSTR)>()
        );
        POINTERS_EQUAL
        (
            ::FindWindowW,
            *atat::FindWindowW.target
            <HWND(*)(const wchar_t*,const wchar_t*)>()
        );
        POINTERS_EQUAL
        (
            ::GetDoubleClickTime,
            *atat::GetDoubleClickTime.target<UINT(*)()>()
        );
        POINTERS_EQUAL
        (
            ::GetForegroundWindow,
            *atat::GetForegroundWindow.target<HWND(*)()>()
        );
        POINTERS_EQUAL
        (
            ::GetLastError,
            *atat::GetLastError.target<DWORD(*)()>()
        );
        POINTERS_EQUAL
        (
            ::GetSystemMetrics,
            *atat::GetSystemMetrics.target<int(*)(int)>()
        );
        POINTERS_EQUAL
        (
            ::GetWindowRect,
            *atat::GetWindowRect.target<BOOL(*)(HWND,LPRECT)>()
        );
        POINTERS_EQUAL
        (
            ::SendInput,
            *atat::SendInput.target<UINT(*)(UINT,LPINPUT,int)>()
        );
        POINTERS_EQUAL
        (
            ::SetConsoleCtrlHandler,
            *atat::SetConsoleCtrlHandler.target
            <BOOL(*)(PHANDLER_ROUTINE,BOOL)>()
        );
        POINTERS_EQUAL
        (
            ::SetEvent,
            *atat::SetEvent.target<BOOL(*)(HANDLE)>()
        );
        POINTERS_EQUAL
        (
            ::WaitForSingleObject,
            *atat::WaitForSingleObject.target<DWORD(*)(HANDLE,DWORD)>()
        );
    });
}

TEST(free,tokenize)
{
    sand_box([] ()
    {
        auto ts=tokenize(" \t \t"," \t");
        CHECK(ts.empty());
    });
    sand_box([] ()
    {
        auto ts=tokenize("a"," \t");
        CHECK_EQUAL(1,ts.size());
        CHECK_EQUAL("a",ts.at(0));
    });
    sand_box([] ()
    {
        auto ts=tokenize("\t\t  a\t\t  "," \t");
        CHECK_EQUAL(1,ts.size());
        CHECK_EQUAL("a",ts.at(0));
    });
    sand_box([] ()
    {
        auto ts=tokenize("a bb\tccc"," \t");
        CHECK_EQUAL(3,ts.size());
        CHECK_EQUAL("a",ts.at(0));
        CHECK_EQUAL("bb",ts.at(1));
        CHECK_EQUAL("ccc",ts.at(2));
    });
}

TEST(free,wait)
{
    sand_box([] ()
    {
        history h;
        atat::CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        atat::CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        atat::WaitForSingleObject=
            [&] (HANDLE hHandle,DWORD dwMilliseconds)->DWORD
            {
                h.calls().push_back(call
                (
                    "WaitForSingleObject",
                    hHandle,
                    dwMilliseconds
                ));
                return WAIT_FAILED;
            };
        atat::GetLastError=
            [&] ()->DWORD
            {
                h.calls().push_back(call("GetLastError"));
                return 34;
            };
        Context::instance()=make_shared<Context>();
        try
        {
            wait(3000);
            FAIL("Don't pass here.");
        } catch(const runtime_error&e)
        {
            STRCMP_EQUAL
            ("function:'WaitForSingleObject':failed(34)",e.what());
        }
        CHECK_EQUAL(2,h.calls().size());
        CHECK_EQUAL
        (call("WaitForSingleObject",(HANDLE)0x12,3000),h.calls().at(0));
        CHECK_EQUAL(call("GetLastError"),h.calls().at(1));
    });
    sand_box([] ()
    {
        history h;
        atat::CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        atat::CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        atat::WaitForSingleObject=
            [&] (HANDLE hHandle,DWORD dwMilliseconds)->DWORD
            {
                h.calls().push_back(call
                (
                    "WaitForSingleObject",
                    hHandle,
                    dwMilliseconds
                ));
                return WAIT_OBJECT_0;
            };
        Context::instance()=make_shared<Context>();
        CHECK_THROWS(aborted_exception,wait(3000));
        CHECK_EQUAL(1,h.calls().size());
        CHECK_EQUAL
        (call("WaitForSingleObject",(HANDLE)0x12,3000),h.calls().at(0));
    });
    sand_box([] ()
    {
        history h;
        atat::CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        atat::CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        atat::WaitForSingleObject=
            [&] (HANDLE hHandle,DWORD dwMilliseconds)->DWORD
            {
                h.calls().push_back(call
                (
                    "WaitForSingleObject",
                    hHandle,
                    dwMilliseconds
                ));
                return WAIT_TIMEOUT;
            };
        Context::instance()=make_shared<Context>();
        wait(3000);
        CHECK_EQUAL(1,h.calls().size());
        CHECK_EQUAL
        (call("WaitForSingleObject",(HANDLE)0x12,3000),h.calls().at(0));
    });
}

TEST(free,wait_active)
{
    sand_box([] ()
    {
        atat::CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        atat::CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        Context::instance()=make_shared<Context>();
        wait_active();
    });
    sand_box([] ()
    {
        history h;
        atat::CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        atat::CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        atat::FindWindowW=
            [&]
            (const wchar_t*lpClassName,const wchar_t*lpWindowName)->HWND
            {
                h.calls().push_back(call
                (
                    "FindWindowW",
                    (const char*)lpClassName,
                    (const char*)lpWindowName
                ));
                return (HWND)0x56;
            };
        atat::GetForegroundWindow=
            [&] ()->HWND
            {
                h.calls().push_back(call("GetForegroundWindow"));
                HWND w;
                switch(h.number_of("GetForegroundWindow"))
                {
                case 1:
                    w=(HWND)0x78;
                    break;
                case 2:
                    w=(HWND)0x56;
                    break;
                default:
                    FAIL("Don't pass here");
                };
                return w;
            };
        atat::GetDoubleClickTime=
            [&] ()->UINT
            {
                h.calls().push_back(call("GetDoubleClickTime"));
                return 100;
            };
        atat::WaitForSingleObject=
            [&] (HANDLE hHandle,DWORD dwMilliseconds)->DWORD
            {
                h.calls().push_back(call
                (
                    "WaitForSingleObject",
                    hHandle,
                    dwMilliseconds
                ));
                return WAIT_TIMEOUT;
            };
        Context::instance()=make_shared<Context>();
        properties().insert(make_pair("target","電卓"));
        wait_active();
        CHECK_EQUAL(6,h.calls().size());
        CHECK_EQUAL
        (
            call
            (
                "FindWindowW",
                (const char*)NULL,
                (const char*)L"電卓"
            ),
            h.calls().at(0)
        );
        CHECK_EQUAL(call("GetForegroundWindow"),h.calls().at(1));
        CHECK_EQUAL(call("GetDoubleClickTime"),h.calls().at(2));
        CHECK_EQUAL(call("WaitForSingleObject",(HANDLE)0x12,100),h.calls().at(3));
        CHECK_EQUAL
        (
            call
            (
                "FindWindowW",
                (const char*)NULL,
                (const char*)L"電卓"
            ),
            h.calls().at(4)
        );
        CHECK_EQUAL(call("GetForegroundWindow"),h.calls().at(5));
    });
}
