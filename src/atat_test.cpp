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

    ct().canceled_event.reset();
    ct().err=nullptr;
    ct().frames.clear();
    ct().in=nullptr;
    ct().index=0;
    ct().out=nullptr;
    ct().properties.clear();

    ct().CloseHandle=nullptr;
    ct().CreateEvent=nullptr;
    ct().FindWindowW=nullptr;
    ct().GetDoubleClickTime=nullptr;
    ct().GetForegroundWindow=nullptr;
    ct().GetLastError=nullptr;
    ct().GetSystemMetrics=nullptr;
    ct().GetWindowRect=nullptr;
    ct().SendInput=nullptr;
    ct().SetConsoleCtrlHandler=nullptr;
    ct().SetEvent=nullptr;
    ct().WaitForSingleObject=nullptr;
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
        auto r=make_shared<Row>("key down A");
        auto kdc=make_shared<KeyDownCommand>(r);
        POINTERS_EQUAL(r.get(),kdc->row());
    });
}

TEST_GROUP(context) {};

TEST(context,setup)
{
    sand_box([] ()
    {
        ct().setup();
        POINTERS_EQUAL
        (
            ::CloseHandle,
            *ct().CloseHandle.target<BOOL(*)(HANDLE)>()
        );
        POINTERS_EQUAL
        (
            ::CreateEvent,
            *ct().CreateEvent.target
            <HANDLE(*)(LPSECURITY_ATTRIBUTES,BOOL,BOOL,LPCTSTR)>()
        );
        POINTERS_EQUAL
        (
            ::FindWindowW,
            *ct().FindWindowW.target
            <HWND(*)(const wchar_t*,const wchar_t*)>()
        );
        POINTERS_EQUAL
        (
            ::GetDoubleClickTime,
            *ct().GetDoubleClickTime.target<UINT(*)()>()
        );
        POINTERS_EQUAL
        (
            ::GetForegroundWindow,
            *ct().GetForegroundWindow.target<HWND(*)()>()
        );
        POINTERS_EQUAL
        (
            ::GetLastError,
            *ct().GetLastError.target<DWORD(*)()>()
        );
        POINTERS_EQUAL
        (
            ::GetSystemMetrics,
            *ct().GetSystemMetrics.target<int(*)(int)>()
        );
        POINTERS_EQUAL
        (
            ::GetWindowRect,
            *ct().GetWindowRect.target<BOOL(*)(HWND,LPRECT)>()
        );
        POINTERS_EQUAL
        (
            ::SendInput,
            *ct().SendInput.target<UINT(*)(UINT,LPINPUT,int)>()
        );
        POINTERS_EQUAL
        (
            ::SetConsoleCtrlHandler,
            *ct().SetConsoleCtrlHandler.target
            <BOOL(*)(PHANDLER_ROUTINE,BOOL)>()
        );
        POINTERS_EQUAL
        (
            ::SetEvent,
            *ct().SetEvent.target<BOOL(*)(HANDLE)>()
        );
        POINTERS_EQUAL
        (
            ::WaitForSingleObject,
            *ct().WaitForSingleObject.target<DWORD(*)(HANDLE,DWORD)>()
        );
        POINTERS_EQUAL(&cin,ct().in);
        POINTERS_EQUAL(&cout,ct().out);
        POINTERS_EQUAL(&cerr,ct().err);
    });
}

TEST_GROUP(Event) {};

TEST(Event,construct)
{
    sand_box([] ()
    {
        history h;
        ct().CreateEvent=
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
        ct().CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        auto e=make_shared<Event>();
        CHECK_EQUAL((HANDLE)0x12,e->handle());
        CHECK_EQUAL(1,h.calls().size());
        CHECK_EQUAL
        (
            call
            (
                "CreateEvent",
                (LPSECURITY_ATTRIBUTES)NULL,
                TRUE,
                FALSE,
                (LPCTSTR)NULL
            ),
            h.calls().at(0)
        );
    });
}

TEST(Event,destruct)
{
    sand_box([] ()
    {
        history h;
        ct().CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        ct().CloseHandle=
            [&] (HANDLE hObject)->BOOL
            {
                h.calls().push_back(call("CloseHandle",hObject));
                return TRUE;
            };
        auto e=make_shared<Event>();
        e.reset();
        CHECK_EQUAL(1,h.calls().size());
        CHECK_EQUAL(call("CloseHandle",(HANDLE)0x12),h.calls().at(0));
    });
}

TEST_GROUP(KeyCommand) {};

TEST(KeyCommand,construct)
{
    sand_box([] ()
    {
        auto r=make_shared<Row>("key");
        try
        {
            auto kdc=make_shared<KeyDownCommand>(r);
            FAIL("Don't pass here.");
        } catch(const runtime_error&e)
        {STRCMP_EQUAL("row:'key':wrong number of tokens",e.what());}
    });
    sand_box([] ()
    {
        auto r=make_shared<Row>("key down A B");
        try
        {
            auto kdc=make_shared<KeyDownCommand>(r);
            FAIL("Don't pass here.");
        } catch(const runtime_error&e)
        {
            STRCMP_EQUAL
            ("row:'key down A B':wrong number of tokens",e.what());
        }
    });
    sand_box([] ()
    {
        auto r=make_shared<Row>("key down HOGE");
        try
        {
            auto kdc=make_shared<KeyDownCommand>(r);
            FAIL("Don't pass here.");
        } catch(const runtime_error&e)
        {STRCMP_EQUAL("key:'HOGE':unknown",e.what());}
    });
    sand_box([] ()
    {
        auto r=make_shared<Row>("key down A");
        auto kdc=make_shared<KeyDownCommand>(r);
    });
    sand_box([] ()
    {
        auto r=make_shared<Row>("key down  a\t");
        auto kdc=make_shared<KeyDownCommand>(r);
    });
    sand_box([] ()
    {
        auto r=make_shared<Row>("key down B");
        auto kdc=make_shared<KeyDownCommand>(r);
    });
}

TEST(KeyCommand,send)
{
    sand_box([] ()
    {
        history h;
        ct().CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        ct().CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        ct().SendInput=
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
        ct().GetDoubleClickTime=[&] ()->UINT {return 100;};
        ct().WaitForSingleObject=
            [&] (HANDLE hHandle,DWORD dwMilliseconds)->DWORD
            {return WAIT_TIMEOUT;};
        ct().canceled_event=make_shared<Event>();
        auto r=make_shared<Row>("key down A");
        auto kdc=make_shared<KeyDownCommand>(r);
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
        ct().CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        ct().CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        ct().SendInput=
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
        ct().GetDoubleClickTime=[&] ()->UINT {return 100;};
        ct().WaitForSingleObject=
            [&] (HANDLE hHandle,DWORD dwMilliseconds)->DWORD
            {return WAIT_TIMEOUT;};
        ct().canceled_event=make_shared<Event>();
        auto r=make_shared<Row>("key up B");
        auto kuc=make_shared<KeyUpCommand>(r);
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
        auto r=make_shared<Row>("key down A");
        auto kdc=make_shared<KeyDownCommand>(r);
        POINTERS_EQUAL(r.get(),kdc->row());
    });
}

TEST(KeyDownCommand,execute)
{
    sand_box([] ()
    {
        history h;
        ct().CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        ct().CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        ct().SendInput=
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
        ct().GetDoubleClickTime=
            [&] ()->UINT
            {
                h.calls().push_back(call("GetDoubleClickTime"));
                return 100;
            };
        ct().WaitForSingleObject=
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
        ct().canceled_event=make_shared<Event>();
        ct().index=3;
        auto r=make_shared<Row>("key down A");
        auto kdc=make_shared<KeyDownCommand>(r);
        kdc->execute();
        CHECK_EQUAL(4,ct().index);
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
        CHECK_EQUAL
        (call("WaitForSingleObject",(HANDLE)0x12,50),h.calls().at(2));
    });
}

TEST_GROUP(KeyPressCommand) {};

TEST(KeyPressCommand,construct)
{
    sand_box([] ()
    {
        auto r=make_shared<Row>("key press A");
        auto kpc=make_shared<KeyPressCommand>(r);
        POINTERS_EQUAL(r.get(),kpc->row());
    });
}

TEST(KeyPressCommand,execute)
{
    sand_box([] ()
    {
        history h;
        ct().CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        ct().CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        ct().SendInput=
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
        ct().GetDoubleClickTime=
            [&] ()->UINT
            {
                h.calls().push_back(call("GetDoubleClickTime"));
                return h.number_of("GetDoubleClickTime")*100;
            };
        ct().WaitForSingleObject=
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
        ct().canceled_event=make_shared<Event>();
        ct().index=3;
        auto r=make_shared<Row>("key press A");
        auto kpc=make_shared<KeyPressCommand>(r);
        kpc->execute();
        CHECK_EQUAL(4,ct().index);
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
        CHECK_EQUAL
        (
            call("WaitForSingleObject",(HANDLE)0x12,50),
            h.calls().at(2)
        );
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
        CHECK_EQUAL
        (
            call("WaitForSingleObject",(HANDLE)0x12,100),
            h.calls().at(5)
        );
    });
}

TEST_GROUP(KeyUpCommand) {};

TEST(KeyUpCommand,construct)
{
    sand_box([] ()
    {
        auto r=make_shared<Row>("key up A");
        auto kuc=make_shared<KeyUpCommand>(r);
        POINTERS_EQUAL(r.get(),kuc->row());
    });
}

TEST(KeyUpCommand,execute)
{
    sand_box([] ()
    {
        history h;
        ct().CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        ct().CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        ct().SendInput=
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
        ct().GetDoubleClickTime=
            [&] ()->UINT
            {
                h.calls().push_back(call("GetDoubleClickTime"));
                return 100;
            };
        ct().WaitForSingleObject=
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
        ct().canceled_event=make_shared<Event>();
        ct().index=3;
        auto r=make_shared<Row>("key up A");
        auto kuc=make_shared<KeyUpCommand>(r);
        kuc->execute();
        CHECK_EQUAL(4,ct().index);
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
        CHECK_EQUAL
        (call("WaitForSingleObject",(HANDLE)0x12,50),h.calls().at(2));
    });
}

TEST_GROUP(MouseButtonClickCommand) {};

TEST(MouseButtonClickCommand,construct)
{
    sand_box([] ()
    {
        auto r=make_shared<Row>("mouse left click");
        auto mbcc=make_shared<MouseButtonClickCommand>(r);
        POINTERS_EQUAL(r.get(),mbcc->row());
    });
}

TEST(MouseButtonClickCommand,execute)
{
    sand_box([] ()
    {
        history h;
        ct().CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        ct().CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        ct().SendInput=
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
        ct().GetDoubleClickTime=
            [&] ()->UINT
            {
                h.calls().push_back(call("GetDoubleClickTime"));
                return h.number_of("GetDoubleClickTime")*100;
            };
        ct().WaitForSingleObject=
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
        ct().canceled_event=make_shared<Event>();
        ct().index=3;
        auto r=make_shared<Row>("mouse left click");
        auto mbcc=make_shared<MouseButtonClickCommand>(r);
        mbcc->execute();
        CHECK_EQUAL(4,ct().index);
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
        CHECK_EQUAL
        (call("WaitForSingleObject",(HANDLE)0x12,50),h.calls().at(2));
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
        CHECK_EQUAL
        (call("WaitForSingleObject",(HANDLE)0x12,100),h.calls().at(5));
    });
}

TEST_GROUP(MouseButtonCommand) {};

TEST(MouseButtonCommand,construct)
{
    sand_box([] ()
    {
        auto r=make_shared<Row>("mouse left down down");
        try
        {
            auto mbdc=make_shared<MouseButtonDownCommand>(r);
            FAIL("Don't pass here.");
        } catch(const runtime_error&e)
        {
            STRCMP_EQUAL
            (
                "row:'mouse left down down':wrong number of tokens",
                e.what()
            );
        }
    });
    sand_box([] ()
    {
        auto r=make_shared<Row>("mouse left down");
        auto mbdc=make_shared<MouseButtonDownCommand>(r);
    });
    sand_box([] ()
    {
        auto r=make_shared<Row>("mouse right up");
        auto mbuc=make_shared<MouseButtonUpCommand>(r);
    });
}

TEST_GROUP(MouseButtonDoubleClickCommand) {};

TEST(MouseButtonDoubleClickCommand,construct)
{
    sand_box([] ()
    {
        auto r=make_shared<Row>("mouse left doubleclick");
        auto mbdcc=make_shared<MouseButtonDoubleClickCommand>(r);
        POINTERS_EQUAL(r.get(),mbdcc->row());
    });
}

TEST(MouseButtonDoubleClickCommand,execute)
{
    sand_box([] ()
    {
        history h;
        ct().CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        ct().CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        ct().SendInput=
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
        ct().GetDoubleClickTime=
            [&] ()->UINT
            {
                h.calls().push_back(call("GetDoubleClickTime"));
                return h.number_of("GetDoubleClickTime")*100;
            };
        ct().WaitForSingleObject=
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
        ct().canceled_event=make_shared<Event>();
        ct().index=3;
        auto r=make_shared<Row>("mouse left doubleclick");
        auto mbdcc=make_shared<MouseButtonDoubleClickCommand>(r);
        mbdcc->execute();
        CHECK_EQUAL(4,ct().index);
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
        CHECK_EQUAL
        (call("WaitForSingleObject",(HANDLE)0x12,25),h.calls().at(2));
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
        CHECK_EQUAL
        (call("WaitForSingleObject",(HANDLE)0x12,50),h.calls().at(5));
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
        CHECK_EQUAL
        (call("WaitForSingleObject",(HANDLE)0x12,75),h.calls().at(8));
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
        CHECK_EQUAL
        (call("WaitForSingleObject",(HANDLE)0x12,100),h.calls().at(11));
    });
}

TEST_GROUP(MouseButtonDownCommand) {};

TEST(MouseButtonDownCommand,construct)
{
    sand_box([] ()
    {
        auto r=make_shared<Row>("mouse left down");
        auto mbdc=make_shared<MouseButtonDownCommand>(r);
        POINTERS_EQUAL(r.get(),mbdc->row());
    });
}

TEST(MouseButtonDownCommand,execute)
{
    sand_box([] ()
    {
        history h;
        ct().CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        ct().CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        ct().SendInput=
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
        ct().GetDoubleClickTime=
            [&] ()->UINT
            {
                h.calls().push_back(call("GetDoubleClickTime"));
                return 100;
            };
        ct().WaitForSingleObject=
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
        ct().canceled_event=make_shared<Event>();
        ct().index=3;
        auto r=make_shared<Row>("mouse left down");
        auto mbdc=make_shared<MouseButtonDownCommand>(r);
        mbdc->execute();
        CHECK_EQUAL(4,ct().index);
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
        CHECK_EQUAL
        (call("WaitForSingleObject",(HANDLE)0x12,50),h.calls().at(2));
    });
}

TEST_GROUP(MouseButtonUpCommand) {};

TEST(MouseButtonUpCommand,construct)
{
    sand_box([] ()
    {
        auto r=make_shared<Row>("mouse left up");
        auto mbuc=make_shared<MouseButtonUpCommand>(r);
        POINTERS_EQUAL(r.get(),mbuc->row());
    });
}

TEST(MouseButtonUpCommand,execute)
{
    sand_box([] ()
    {
        history h;
        ct().CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        ct().CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        ct().SendInput=
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
        ct().GetDoubleClickTime=
            [&] ()->UINT
            {
                h.calls().push_back(call("GetDoubleClickTime"));
                return 100;
            };
        ct().WaitForSingleObject=
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
        ct().canceled_event=make_shared<Event>();
        ct().index=3;
        auto r=make_shared<Row>("mouse left up");
        auto mbuc=make_shared<MouseButtonUpCommand>(r);
        mbuc->execute();
        CHECK_EQUAL(4,ct().index);
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
        CHECK_EQUAL
        (call("WaitForSingleObject",(HANDLE)0x12,50),h.calls().at(2));
    });
}

TEST_GROUP(MouseCommand) {};

TEST(MouseCommand,send)
{
    sand_box([] ()
    {
        history h;
        ct().SendInput=
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
        ct().GetLastError=
            [&] ()->DWORD
            {
                h.calls().push_back(call("GetLastError"));
                return 34;
            };
        auto r=make_shared<Row>("mouse left down");
        auto mbdc=make_shared<MouseButtonDownCommand>(r);
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
        ct().CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        ct().CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        ct().SendInput=
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
        ct().GetDoubleClickTime=[&] ()->UINT {return 100;};
        ct().WaitForSingleObject=
            [&] (HANDLE hHandle,DWORD dwMilliseconds)->DWORD
            {return WAIT_TIMEOUT;};
        ct().canceled_event=make_shared<Event>();
        ct().index=3;
        auto r=make_shared<Row>("mouse left down");
        auto mbdc=make_shared<MouseButtonDownCommand>(r);
        mbdc->execute();
        CHECK_EQUAL(4,ct().index);
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
        auto r=make_shared<Row>("mouse move");
        try
        {
            auto mmc=make_shared<MouseMoveCommand>(r);
            FAIL("Don't pass here.");
        } catch(const runtime_error&e)
        {
            STRCMP_EQUAL
            ("row:'mouse move':wrong number of tokens",e.what());
        }
    });
    sand_box([] ()
    {
        auto r=make_shared<Row>("mouse move 100");
        try
        {
            auto mmc=make_shared<MouseMoveCommand>(r);
            FAIL("Don't pass here.");
        } catch(const runtime_error&e)
        {
            STRCMP_EQUAL
            ("row:'mouse move 100':wrong number of tokens",e.what());
        }
    });
    sand_box([] ()
    {
        auto r=make_shared<Row>("mouse move 100 200 300");
        try
        {
            auto mmc=make_shared<MouseMoveCommand>(r);
            FAIL("Don't pass here.");
        } catch(const runtime_error&e)
        {
            STRCMP_EQUAL
            (
                "row:'mouse move 100 200 300':wrong number of tokens",
                e.what()
            );
        }
    });
    sand_box([] ()
    {
        auto r=make_shared<Row>("mouse move abc 200");
        try
        {
            auto mmc=make_shared<MouseMoveCommand>(r);
            FAIL("Don't pass here.");
        } catch(const runtime_error&e)
        {STRCMP_EQUAL("number:'abc':invalid format",e.what());}
    });
    sand_box([] ()
    {
        auto r=make_shared<Row>("mouse move 100 def");
        try
        {
            auto mmc=make_shared<MouseMoveCommand>(r);
            FAIL("Don't pass here.");
        } catch(const runtime_error&e)
        {STRCMP_EQUAL("number:'def':invalid format",e.what());}
    });
    sand_box([] ()
    {
        auto r=make_shared<Row>("mouse move 100 200");
        auto mmc=make_shared<MouseMoveCommand>(r);
    });
}

TEST(MouseMoveCommand,execute)
{
    sand_box([] ()
    {
        history h;
        ct().GetSystemMetrics=
            [&] (int nIndex)->int
            {
                h.calls().push_back(call("GetSystemMetrics",nIndex));
                return 0;
            };
        auto r=make_shared<Row>("mouse move 100 200");
        auto mmc=make_shared<MouseMoveCommand>(r);
        try
        {
            mmc->execute();
            FAIL("Don't pass here.");
        } catch(const runtime_error&e)
        {STRCMP_EQUAL("function:'GetSystemMetrics':failed",e.what());}
        CHECK_EQUAL(1,h.calls().size());
        CHECK_EQUAL(call("GetSystemMetrics",SM_CXSCREEN),h.calls().at(0));
    });
    sand_box([] ()
    {
        history h;
        ct().GetSystemMetrics=
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
                    FAIL("Don't pass here.");
                };
                return result;
            };
        auto r=make_shared<Row>("mouse move 100 200");
        auto mmc=make_shared<MouseMoveCommand>(r);
        try
        {
            mmc->execute();
            FAIL("Don't pass here.");
        } catch(const runtime_error&e)
        {STRCMP_EQUAL("function:'GetSystemMetrics':failed",e.what());}
        CHECK_EQUAL(2,h.calls().size());
        CHECK_EQUAL(call("GetSystemMetrics",SM_CXSCREEN),h.calls().at(0));
        CHECK_EQUAL(call("GetSystemMetrics",SM_CYSCREEN),h.calls().at(1));
    });
    sand_box([] ()
    {
        history h;
        ct().GetSystemMetrics=
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
                    FAIL("Don't pass here.");
                };
                return result;
            };
        ct().SendInput=
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
        ct().index=3;
        auto r=make_shared<Row>("mouse move 100 200");
        auto mmc=make_shared<MouseMoveCommand>(r);
        mmc->execute();
        CHECK_EQUAL(4,ct().index);
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
        ct().FindWindowW=
            [&]
            (const wchar_t*lpClassName,const wchar_t*lpWindowName)->HWND
            {return (HWND)0x12;};
        ct().GetWindowRect=
            [&] (HWND hWnd,LPRECT lpRect)->BOOL
            {
                h.calls().push_back(call("GetWindowRect",hWnd));
                return FALSE;
            };
        ct().GetLastError=
            [&] ()->DWORD
            {
                h.calls().push_back(call("GetLastError"));
                return 34;
            };
        ct().properties.insert({"target","電卓"});
        auto r=make_shared<Row>("mouse move 100 200");
        auto mmc=make_shared<MouseMoveCommand>(r);
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
        ct().FindWindowW=
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
        ct().GetWindowRect=
            [&] (HWND hWnd,LPRECT lpRect)->BOOL
            {
                h.calls().push_back(call("GetWindowRect",hWnd));
                lpRect->left=30;
                lpRect->top=60;
                return TRUE;
            };
        ct().GetSystemMetrics=
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
                    FAIL("Don't pass here.");
                };
                return result;
            };
        ct().SendInput=
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
        ct().properties.insert({"target","電卓"});
        ct().index=3;
        auto r=make_shared<Row>("mouse move 100 200");
        auto mmc=make_shared<MouseMoveCommand>(r);
        mmc->execute();
        CHECK_EQUAL(4,ct().index);
        CHECK_EQUAL(5,h.calls().size());
        CHECK_EQUAL
        (
            call("FindWindowW",(const char*)NULL,(const char*)L"電卓"),
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
    sand_box([] ()
    {
        history h;
        ct().FindWindowW=
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
        ct().GetWindowRect=
            [&] (HWND hWnd,LPRECT lpRect)->BOOL
            {
                h.calls().push_back(call("GetWindowRect",hWnd));
                lpRect->left=100;
                lpRect->top=200;
                return TRUE;
            };
        ct().GetSystemMetrics=
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
                    FAIL("Don't pass here.");
                };
                return result;
            };
        ct().SendInput=
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
        ct().properties.insert({"target","電卓"});
        ct().index=3;
        auto r=make_shared<Row>("mouse move -30 -60");
        auto mmc=make_shared<MouseMoveCommand>(r);
        mmc->execute();
        CHECK_EQUAL(4,ct().index);
        CHECK_EQUAL(5,h.calls().size());
        CHECK_EQUAL
        (
            call("FindWindowW",(const char*)NULL,(const char*)L"電卓"),
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
                (70<<16)/400,
                (140<<16)/300,
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
        auto r=make_shared<Row>("mouse wheel");
        try
        {
            auto mwc=make_shared<MouseWheelCommand>(r);
            FAIL("Don't pass here.");
        } catch(const runtime_error&e)
        {
            STRCMP_EQUAL
            ("row:'mouse wheel':wrong number of tokens",e.what());
        }
    });
    sand_box([] ()
    {
        auto r=make_shared<Row>("mouse wheel 10 20");
        try
        {
            auto mwc=make_shared<MouseWheelCommand>(r);
            FAIL("Don't pass here.");
        } catch(const runtime_error&e)
        {
            STRCMP_EQUAL
            ("row:'mouse wheel 10 20':wrong number of tokens",e.what());
        }
    });
    sand_box([] ()
    {
        auto r=make_shared<Row>("mouse wheel ab");
        try
        {
            auto mwc=make_shared<MouseWheelCommand>(r);
            FAIL("Don't pass here.");
        } catch(const runtime_error&e)
        {STRCMP_EQUAL("number:'ab':invalid format",e.what());}
    });
    sand_box([] ()
    {
        auto r=make_shared<Row>("mouse wheel 10");
        auto mwc=make_shared<MouseWheelCommand>(r);
    });
}

TEST(MouseWheelCommand,execute)
{
    sand_box([] ()
    {
        history h;
        ct().SendInput=
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
        ct().index=3;
        auto r=make_shared<Row>("mouse wheel 10");
        auto mwc=make_shared<MouseWheelCommand>(r);
        mwc->execute();
        CHECK_EQUAL(4,ct().index);
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
        ct().SendInput=
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
        ct().index=3;
        auto r=make_shared<Row>("mouse wheel -10");
        auto mwc=make_shared<MouseWheelCommand>(r);
        mwc->execute();
        CHECK_EQUAL(4,ct().index);
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
        auto r=make_shared<Row>("loop begin 5 5");
        try
        {
            auto lbc=make_shared<LoopBeginCommand>(r);
            FAIL("Don't pass here.");
        } catch(const runtime_error&e)
        {
            STRCMP_EQUAL
            ("row:'loop begin 5 5':wrong number of tokens",e.what());
        }
    });
    sand_box([] ()
    {
        auto r=make_shared<Row>("loop begin a");
        try
        {
            auto lbc=make_shared<LoopBeginCommand>(r);
            FAIL("Don't pass here.");
        } catch(const runtime_error&e)
        {STRCMP_EQUAL("number:'a':invalid format",e.what());}
    });
    sand_box([] ()
    {
        auto r=make_shared<Row>("loop begin");
        auto lbc=make_shared<LoopBeginCommand>(r);
    });
    sand_box([] ()
    {
        auto r=make_shared<Row>("loop begin 5");
        auto lbc=make_shared<LoopBeginCommand>(r);
    });
}

TEST(LoopBeginCommand,execute)
{
    sand_box([] ()
    {
        ct().index=3;
        auto r=make_shared<Row>("loop begin");
        auto lbc=make_shared<LoopBeginCommand>(r);
        lbc->execute();
        CHECK_EQUAL(4,ct().index);
        CHECK_EQUAL(1,ct().frames.size());
        CHECK_EQUAL(0,ct().frames.back().counter);
        CHECK_EQUAL(4,ct().frames.back().entry);
        CHECK_EQUAL(0,ct().frames.back().number);
    });
    sand_box([] ()
    {
        ct().index=3;
        auto r=make_shared<Row>("loop begin 5");
        auto lbc=make_shared<LoopBeginCommand>(r);
        lbc->execute();
        CHECK_EQUAL(4,ct().index);
        CHECK_EQUAL(1,ct().frames.size());
        CHECK_EQUAL(0,ct().frames.back().counter);
        CHECK_EQUAL(4,ct().frames.back().entry);
        CHECK_EQUAL(5,ct().frames.back().number);
    });
}

TEST_GROUP(LoopEndCommand) {};

TEST(LoopEndCommand,construct)
{
    sand_box([] ()
    {
        auto r=make_shared<Row>("loop end end");
        try
        {
            auto lec=make_shared<LoopEndCommand>(r);
            FAIL("Don't pass here.");
        } catch(const runtime_error&e)
        {
            STRCMP_EQUAL
            ("row:'loop end end':wrong number of tokens",e.what());
        }
    });
    sand_box([] ()
    {
        auto r=make_shared<Row>("loop end");
        auto lec=make_shared<LoopEndCommand>(r);
    });
}

TEST(LoopEndCommand,execute)
{
    sand_box([] ()
    {
        ct().frames.push_back({0,0,0});
        ct().index=3;
        auto r=make_shared<Row>("loop end");
        auto lec=make_shared<LoopEndCommand>(r);
        try
        {
            lec->execute();
            FAIL("Don't pass here.");
        } catch(const runtime_error&e)
        {STRCMP_EQUAL("loop end:no corresponding begin",e.what());}
    });
    sand_box([] ()
    {
        ct().frames.push_back({0,0,0});
        ct().index=6;
        ct().frames.push_back({3,4,5});
        auto r=make_shared<Row>("loop end");
        auto lec=make_shared<LoopEndCommand>(r);
        lec->execute();
        CHECK_EQUAL(4,ct().index);
        CHECK_EQUAL(2,ct().frames.size());
        CHECK_EQUAL(4,ct().frames.back().counter);
        CHECK_EQUAL(4,ct().frames.back().entry);
        CHECK_EQUAL(5,ct().frames.back().number);
    });
    sand_box([] ()
    {
        ct().frames.push_back({0,0,0});
        ct().index=6;
        ct().frames.push_back({4,4,5});
        auto r=make_shared<Row>("loop end");
        auto lec=make_shared<LoopEndCommand>(r);
        lec->execute();
        CHECK_EQUAL(7,ct().index);
        CHECK_EQUAL(1,ct().frames.size());
    });
}

TEST_GROUP(Row) {};

TEST(Row,construct)
{
    sand_box([] ()
    {
        auto r=make_shared<Row>("abc def");
        CHECK_EQUAL("abc def",r->description());
        CHECK_EQUAL(2,r->tokens().size());
        CHECK_EQUAL("abc",r->tokens().at(0));
        CHECK_EQUAL("def",r->tokens().at(1));
    });
    sand_box([] ()
    {
        auto r=make_shared<Row>("abc#def");
        CHECK_EQUAL("abc#def",r->description());
        CHECK_EQUAL(1,r->tokens().size());
        CHECK_EQUAL("abc",r->tokens().at(0));
    });
}

TEST_GROUP(SleepCommand) {};

TEST(SleepCommand,construct)
{
    sand_box([] ()
    {
        auto r=make_shared<Row>("sleep");
        try
        {
            auto sc=make_shared<SleepCommand>(r);
            FAIL("Don't pass here.");
        } catch(const runtime_error&e)
        {
            STRCMP_EQUAL
            ("row:'sleep':wrong number of tokens",e.what());
        }
    });
    sand_box([] ()
    {
        auto r=make_shared<Row>("sleep 1000 2000");
        try
        {
            auto sc=make_shared<SleepCommand>(r);
            FAIL("Don't pass here.");
        } catch(const runtime_error&e)
        {
            STRCMP_EQUAL
            ("row:'sleep 1000 2000':wrong number of tokens",e.what());
        }
    });
    sand_box([] ()
    {
        auto r=make_shared<Row>("sleep abcd");
        try
        {
            auto sc=make_shared<SleepCommand>(r);
            FAIL("Don't pass here.");
        } catch(const runtime_error&e)
        {STRCMP_EQUAL("number:'abcd':invalid format",e.what());}
    });
    sand_box([] ()
    {
        auto r=make_shared<Row>("sleep 1000");
        auto sc=make_shared<SleepCommand>(r);
    });
}

TEST(SleepCommand,execute)
{
    sand_box([] ()
    {
        history h;
        ct().CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        ct().CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        ct().WaitForSingleObject=
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
        ct().canceled_event=make_shared<Event>();
        ct().index=3;
        auto r=make_shared<Row>("sleep 1000");
        auto sc=make_shared<SleepCommand>(r);
        sc->execute();
        CHECK_EQUAL(4,ct().index);
        CHECK_EQUAL(1,h.calls().size());
        CHECK_EQUAL
        (call("WaitForSingleObject",(HANDLE)0x12,1000),h.calls().at(0));
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
        ct().CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        ct().CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        ct().SetEvent=
            [&] (HANDLE hEvent)->BOOL
            {
                h.calls().push_back(call("SetEvent",hEvent));
                return TRUE;
            };
        ct().canceled_event=make_shared<Event>();
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

TEST(free,execute)
{
    sand_box([] ()
    {
        ct().CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        ct().CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        ct().canceled_event=make_shared<Event>();
        static const char*argv[]={"test","help"};
        istringstream in("");
        ostringstream out,err;
        ct().in=&in;
        ct().out=&out;
        ct().err=&err;
        int result=execute(2,(char**)argv);
        CHECK(ct().properties.find("help")!=ct().properties.end());
        CHECK_EQUAL(0,result);
        CHECK_EQUAL("usage",out.str().substr(0,5));
        CHECK_EQUAL("",err.str());
    });
    sand_box([] ()
    {
        history h;
        ct().CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        ct().CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        ct().SetConsoleCtrlHandler=
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
        ct().GetLastError=
            [&] ()->DWORD
            {
                h.calls().push_back(call("GetLastError"));
                return 34;
            };
        ct().canceled_event=make_shared<Event>();
        static const char*argv[]={"test"};
        istringstream in("");
        ostringstream out,err;
        ct().in=&in;
        ct().out=&out;
        ct().err=&err;
        int result=execute(2,(char**)argv);
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
        ct().CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        ct().CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        ct().SetConsoleCtrlHandler=
            [&] (PHANDLER_ROUTINE HandlerRoutine,BOOL Add)->BOOL
            {return TRUE;};
        ct().WaitForSingleObject=
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
                    FAIL("Don't pass here.");
                };
                return result;
            };
        ct().canceled_event=make_shared<Event>();
        static const char*argv[]={"test"};
        istringstream in
        (
            "sleep 1000\n"
            "sleep 2000\n"
            "sleep 3000\n"
        );
        ostringstream out,err;
        ct().in=&in;
        ct().out=&out;
        ct().err=&err;
        int result=execute(1,(char**)argv);
        CHECK_EQUAL(0,result);
        CHECK_EQUAL
        (
            "sleep 1000\n"
            "sleep 2000\n",
            out.str()
        );
        CHECK_EQUAL("",err.str());
        CHECK_EQUAL(1,ct().index);
        CHECK_EQUAL(2,h.calls().size());
        CHECK_EQUAL
        (call("WaitForSingleObject",(HANDLE)0x12,1000),h.calls().at(0));
        CHECK_EQUAL
        (call("WaitForSingleObject",(HANDLE)0x12,2000),h.calls().at(1));
    });
}

TEST(free,find_target)
{
    sand_box([] ()
    {
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
        ct().FindWindowW=
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
        ct().properties.insert({"target","電卓"});
        try
        {
            find_target();
            FAIL("Don't pass here.");
        } catch(const runtime_error&e)
        {STRCMP_EQUAL("target:'電卓':not found",e.what());}
        CHECK_EQUAL(1,h.calls().size());
        CHECK_EQUAL
        (
            call("FindWindowW",(const char*)NULL,(const char*)L"電卓"),
            h.calls().at(0)
        );
    });
    sand_box([] ()
    {
        history h;
        ct().FindWindowW=
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
        ct().properties.insert({"target","電卓"});
        HWND w=find_target();
        CHECK_EQUAL((HWND)0x56,w);
        CHECK_EQUAL(1,h.calls().size());
        CHECK_EQUAL
        (
            call("FindWindowW",(const char*)NULL,(const char*)L"電卓"),
            h.calls().at(0)
        );
    });
}

TEST(free,frame_begin)
{
    sand_box([] ()
    {
        ct().index=4;
        frame_begin(0);
        CHECK_EQUAL(4,ct().index);
        CHECK_EQUAL(1,ct().frames.size());
        CHECK_EQUAL(0,ct().frames.back().counter);
        CHECK_EQUAL(4,ct().frames.back().entry);
        CHECK_EQUAL(0,ct().frames.back().number);
    });
    sand_box([] ()
    {
        ct().frames.push_back({0,0,0});
        ct().index=6;
        frame_begin(3);
        CHECK_EQUAL(6,ct().index);
        CHECK_EQUAL(2,ct().frames.size());
        CHECK_EQUAL(0,ct().frames.back().counter);
        CHECK_EQUAL(6,ct().frames.back().entry);
        CHECK_EQUAL(3,ct().frames.back().number);
    });
}

TEST(free,frame_end)
{
    sand_box([] ()
    {
        ct().index=6;
        ct().frames.push_back({0,4,0});
        CHECK(frame_end());
        CHECK_EQUAL(4,ct().index);
        CHECK_EQUAL(1,ct().frames.size());
        CHECK_EQUAL(0,ct().frames.back().counter);
        CHECK_EQUAL(4,ct().frames.back().entry);
        CHECK_EQUAL(0,ct().frames.back().number);
    });
    sand_box([] ()
    {
        ct().index=6;
        ct().frames.push_back({3,4,5});
        CHECK(frame_end());
        CHECK_EQUAL(4,ct().index);
        CHECK_EQUAL(1,ct().frames.size());
        CHECK_EQUAL(4,ct().frames.back().counter);
        CHECK_EQUAL(4,ct().frames.back().entry);
        CHECK_EQUAL(5,ct().frames.back().number);
    });
    sand_box([] ()
    {
        ct().index=6;
        ct().frames.push_back({4,4,5});
        CHECK_FALSE(frame_end());
        CHECK_EQUAL(6,ct().index);
        CHECK(ct().frames.empty());
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
        STRCMP_EQUAL
        (
            (const char*)L"いろは",
            (const char*)multi_to_wide("いろは",CP_UTF8).get()
        );
        STRCMP_EQUAL
        (
            (const char*)L"にほへと",
            (const char*)multi_to_wide("にほへと",CP_UTF8).get()
        );
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
                [] (const shared_ptr<Row>&row)->shared_ptr<Command>
                {return make_shared<KeyDownCommand>(row);}
            },
            {
                "up",
                [] (const shared_ptr<Row>&row)->shared_ptr<Command>
                {return make_shared<KeyUpCommand>(row);}
            },
        });
        auto r=make_shared<Row>("");
        CHECK(dynamic_cast<NullCommand*>(new_command(fs,r,0).get()));
    });
    sand_box([] ()
    {
        map<string,COMMAND_FACTORY> fs(
        {
            {
                "down",
                [] (const shared_ptr<Row>&row)->shared_ptr<Command>
                {return make_shared<KeyDownCommand>(row);}
            },
            {
                "up",
                [] (const shared_ptr<Row>&row)->shared_ptr<Command>
                {return make_shared<KeyUpCommand>(row);}
            },
        });
        auto r=make_shared<Row>("key down A");
        try
        {
            auto c=new_command(fs,r,3);
            FAIL("Don't pass here.");
        } catch(const runtime_error&e)
        {STRCMP_EQUAL("row:'key down A':few switches",e.what());}
    });
    sand_box([] ()
    {
        map<string,COMMAND_FACTORY> fs(
        {
            {
                "down",
                [] (const shared_ptr<Row>&row)->shared_ptr<Command>
                {return make_shared<KeyDownCommand>(row);}
            },
            {
                "up",
                [] (const shared_ptr<Row>&row)->shared_ptr<Command>
                {return make_shared<KeyUpCommand>(row);}
            },
        });
        auto r=make_shared<Row>("key press A");
        try
        {
            auto c=new_command(fs,r,1);
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
                [] (const shared_ptr<Row>&row)->shared_ptr<Command>
                {return make_shared<KeyDownCommand>(row);}
            },
            {
                "up",
                [] (const shared_ptr<Row>&row)->shared_ptr<Command>
                {return make_shared<KeyUpCommand>(row);}
            },
        });
        auto r=make_shared<Row>("key down A");
        CHECK(dynamic_cast<KeyDownCommand*>(new_command(fs,r,1).get()));
    });
    sand_box([] ()
    {
        map<string,COMMAND_FACTORY> fs(
        {
            {
                "down",
                [] (const shared_ptr<Row>&row)->shared_ptr<Command>
                {return make_shared<KeyDownCommand>(row);}
            },
            {
                "up",
                [] (const shared_ptr<Row>&row)->shared_ptr<Command>
                {return make_shared<KeyUpCommand>(row);}
            },
        });
        auto r=make_shared<Row>("key DoWn A");
        CHECK(dynamic_cast<KeyDownCommand*>(new_command(fs,r,1).get()));
    });
    sand_box([] ()
    {
        map<string,COMMAND_FACTORY> fs(
        {
            {
                "down",
                [] (const shared_ptr<Row>&row)->shared_ptr<Command>
                {return make_shared<KeyDownCommand>(row);}
            },
            {
                "up",
                [] (const shared_ptr<Row>&row)->shared_ptr<Command>
                {return make_shared<KeyUpCommand>(row);}
            },
        });
        auto r=make_shared<Row>("key up A");
        CHECK(dynamic_cast<KeyUpCommand*>(new_command(fs,r,1).get()));
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

TEST(free,parse_script)
{
    sand_box([] ()
    {
        istringstream in
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
            "sleep 1000\n"
            " \n"
        );
        ct().in=&in;
        auto cs=parse_script();
        CHECK_EQUAL(19,cs.size());
        CHECK(dynamic_cast<KeyDownCommand*>(cs.at(0).get()));
        CHECK(dynamic_cast<KeyUpCommand*>(cs.at(1).get()));
        CHECK(dynamic_cast<KeyPressCommand*>(cs.at(2).get()));
        CHECK(dynamic_cast<MouseButtonDownCommand*>(cs.at(3).get()));
        CHECK(dynamic_cast<MouseButtonUpCommand*>(cs.at(4).get()));
        CHECK(dynamic_cast<MouseButtonClickCommand*>(cs.at(5).get()));
        CHECK
        (dynamic_cast<MouseButtonDoubleClickCommand*>(cs.at(6).get()));
        CHECK(dynamic_cast<MouseButtonDownCommand*>(cs.at(7).get()));
        CHECK(dynamic_cast<MouseButtonUpCommand*>(cs.at(8).get()));
        CHECK(dynamic_cast<MouseButtonClickCommand*>(cs.at(9).get()));
        CHECK
        (dynamic_cast<MouseButtonDoubleClickCommand*>(cs.at(10).get()));
        CHECK(dynamic_cast<MouseButtonDownCommand*>(cs.at(11).get()));
        CHECK(dynamic_cast<MouseButtonUpCommand*>(cs.at(12).get()));
        CHECK(dynamic_cast<MouseButtonClickCommand*>(cs.at(13).get()));
        CHECK
        (dynamic_cast<MouseButtonDoubleClickCommand*>(cs.at(14).get()));
        CHECK(dynamic_cast<LoopBeginCommand*>(cs.at(15).get()));
        CHECK(dynamic_cast<LoopEndCommand*>(cs.at(16).get()));
        CHECK(dynamic_cast<SleepCommand*>(cs.at(17).get()));
        CHECK(dynamic_cast<NullCommand*>(cs.at(18).get()));
        CHECK_EQUAL(19,ct().index);
    });
}

TEST(free,run)
{
    sand_box([] ()
    {
        history h;
        ct().CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        ct().CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        ct().WaitForSingleObject=
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
        ct().canceled_event=make_shared<Event>();
        static const char*argv[]={"test"};
        istringstream in
        (
            "sleep 1000\n"
            "loop begin\n"
            "sleep 2000\n"
        );
        ostringstream out;
        ct().properties=parse_properties(1,(char**)argv);
        ct().in=&in;
        ct().out=&out;
        try
        {
            atat::run(parse_script());
            FAIL("Don't pass here.");
        } catch(const runtime_error&e)
        {STRCMP_EQUAL("loop begin:no corresponding end",e.what());}
        CHECK_EQUAL
        (
            "sleep 1000\n"
            "loop begin\n"
            "sleep 2000\n",
            out.str()
        );
        CHECK_EQUAL(1,ct().index);
        CHECK_EQUAL(2,h.calls().size());
        CHECK_EQUAL
        (call("WaitForSingleObject",(HANDLE)0x12,1000),h.calls().at(0));
        (call("WaitForSingleObject",(HANDLE)0x12,2000),h.calls().at(1));
    });
    sand_box([] ()
    {
        history h;
        ct().CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        ct().CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        ct().FindWindowW=
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
        ct().GetForegroundWindow=
            [&] ()->HWND
            {
                h.calls().push_back(call("GetForegroundWindow"));
                return (HWND)0x56;
            };
        ct().WaitForSingleObject=
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
        ct().canceled_event=make_shared<Event>();
        static const char*argv[]={"test","target=GAME"};
        istringstream in
        (
            "loop begin 2\n"
            "sleep 1000\n"
            "loop end\n"
        );
        ostringstream out;
        ct().properties=parse_properties(2,(char**)argv);
        ct().in=&in;
        ct().out=&out;
        atat::run(parse_script());
        CHECK_EQUAL
        (
            "loop begin 2\n"
            "sleep 1000\n"
            "loop end\n"
            "sleep 1000\n"
            "loop end\n",
            out.str()
        );
        CHECK_EQUAL(3,ct().index);
        CHECK_EQUAL(12,h.calls().size());
        CHECK_EQUAL
        (
            call("FindWindowW",(const char*)NULL,(const char*)L"GAME"),
            h.calls().at(0)
        );
        CHECK_EQUAL(call("GetForegroundWindow"),h.calls().at(1));
        CHECK_EQUAL
        (
            call("FindWindowW",(const char*)NULL,(const char*)L"GAME"),
            h.calls().at(2)
        );
        CHECK_EQUAL(call("GetForegroundWindow"),h.calls().at(3));
        CHECK_EQUAL
        (call("WaitForSingleObject",(HANDLE)0x12,1000),h.calls().at(4));
        CHECK_EQUAL
        (
            call("FindWindowW",(const char*)NULL,(const char*)L"GAME"),
            h.calls().at(5)
        );
        CHECK_EQUAL(call("GetForegroundWindow"),h.calls().at(6));
        CHECK_EQUAL
        (
            call("FindWindowW",(const char*)NULL,(const char*)L"GAME"),
            h.calls().at(7)
        );
        CHECK_EQUAL(call("GetForegroundWindow"),h.calls().at(8));
        CHECK_EQUAL
        (call("WaitForSingleObject",(HANDLE)0x12,1000),h.calls().at(9));
        CHECK_EQUAL
        (
            call("FindWindowW",(const char*)NULL,(const char*)L"GAME"),
            h.calls().at(10)
        );
        CHECK_EQUAL(call("GetForegroundWindow"),h.calls().at(11));
    });
    sand_box([] ()
    {
        history h;
        ct().CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        ct().CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        ct().WaitForSingleObject=
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
                case 1:case 2:
                    result=WAIT_TIMEOUT;
                    break;
                case 3:
                    result=WAIT_OBJECT_0;
                    break;
                default:
                    FAIL("Don't pass here.");
                };
                return result;
            };
        ct().canceled_event=make_shared<Event>();
        static const char*argv[]={"test","repeat"};
        istringstream in("sleep 1000\n");
        ostringstream out;
        ct().properties=parse_properties(2,(char**)argv);
        ct().in=&in;
        ct().out=&out;
        CHECK_THROWS(canceled_exception,atat::run(parse_script()));
        CHECK_EQUAL
        (
            "sleep 1000\n"
            "sleep 1000\n"
            "sleep 1000\n",
            out.str()
        );
        CHECK_EQUAL(0,ct().index);
        CHECK_EQUAL(3,h.calls().size());
        CHECK_EQUAL
        (call("WaitForSingleObject",(HANDLE)0x12,1000),h.calls().at(0));
        CHECK_EQUAL
        (call("WaitForSingleObject",(HANDLE)0x12,1000),h.calls().at(1));
        CHECK_EQUAL
        (call("WaitForSingleObject",(HANDLE)0x12,1000),h.calls().at(2));
    });
    sand_box([] ()
    {
        history h;
        ct().CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        ct().CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        ct().WaitForSingleObject=
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
        ct().canceled_event=make_shared<Event>();
        static const char*argv[]={"test","repeat=2"};
        istringstream in("sleep 1000\n");
        ostringstream out;
        ct().properties=parse_properties(2,(char**)argv);
        ct().in=&in;
        ct().out=&out;
        atat::run(parse_script());
        CHECK_EQUAL
        (
            "sleep 1000\n"
            "sleep 1000\n",
            out.str()
        );
        CHECK_EQUAL(1,ct().index);
        CHECK_EQUAL(2,h.calls().size());
        CHECK_EQUAL
        (call("WaitForSingleObject",(HANDLE)0x12,1000),h.calls().at(0));
        CHECK_EQUAL
        (call("WaitForSingleObject",(HANDLE)0x12,1000),h.calls().at(1));
    });
    sand_box([] ()
    {
        history h;
        ct().CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        ct().CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        ct().WaitForSingleObject=
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
        ct().canceled_event=make_shared<Event>();
        static const char*argv[]={"test","ready=3000"};
        istringstream in("sleep 1000\n");
        ostringstream out;
        ct().properties=parse_properties(2,(char**)argv);
        ct().in=&in;
        ct().out=&out;
        atat::run(parse_script());
        CHECK_EQUAL
        (
            "sleep 1000\n",
            out.str()
        );
        CHECK_EQUAL(1,ct().index);
        CHECK_EQUAL(2,h.calls().size());
        CHECK_EQUAL
        (call("WaitForSingleObject",(HANDLE)0x12,3000),h.calls().at(0));
        CHECK_EQUAL
        (call("WaitForSingleObject",(HANDLE)0x12,1000),h.calls().at(1));
    });
}

TEST(free,to_number)
{
    sand_box([] ()
    {
        CHECK_EQUAL(0,to_number(""));
        try
        {
            to_number("a12");
            FAIL("Don't pass here.");
        } catch(const runtime_error&e)
        {STRCMP_EQUAL("number:'a12':invalid format",e.what());}
        try
        {
            to_number("12a");
            FAIL("Don't pass here.");
        } catch(const runtime_error&e)
        {STRCMP_EQUAL("number:'12a':invalid format",e.what());}
        CHECK_EQUAL(12,to_number("12"));
        CHECK_EQUAL(-12,to_number("-12"));
        try
        {
            to_number("0812");
            FAIL("Don't pass here.");
        } catch(const runtime_error&e)
        {STRCMP_EQUAL("number:'0812':invalid format",e.what());}
        try
        {
            to_number("0128");
            FAIL("Don't pass here.");
        } catch(const runtime_error&e)
        {STRCMP_EQUAL("number:'0128':invalid format",e.what());}
        CHECK_EQUAL(012,to_number("012"));
        try
        {
            to_number("0xg12");
            FAIL("Don't pass here.");
        } catch(const runtime_error&e)
        {STRCMP_EQUAL("number:'0xg12':invalid format",e.what());}
        try
        {
            to_number("0x12g");
            FAIL("Don't pass here.");
        } catch(const runtime_error&e)
        {STRCMP_EQUAL("number:'0x12g':invalid format",e.what());}
        CHECK_EQUAL(0x12,to_number("0x12"));
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
        ct().CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        ct().CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        ct().WaitForSingleObject=
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
        ct().GetLastError=
            [&] ()->DWORD
            {
                h.calls().push_back(call("GetLastError"));
                return 34;
            };
        ct().canceled_event=make_shared<Event>();
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
        ct().CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        ct().CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        ct().WaitForSingleObject=
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
        ct().canceled_event=make_shared<Event>();
        CHECK_THROWS(canceled_exception,wait(3000));
        CHECK_EQUAL(1,h.calls().size());
        CHECK_EQUAL
        (call("WaitForSingleObject",(HANDLE)0x12,3000),h.calls().at(0));
    });
    sand_box([] ()
    {
        history h;
        ct().CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        ct().CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        ct().WaitForSingleObject=
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
        ct().canceled_event=make_shared<Event>();
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
        ct().CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        ct().CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        wait_active();
    });
    sand_box([] ()
    {
        history h;
        ct().CreateEvent=
            [&]
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,
                BOOL bManualReset,
                BOOL bInitialState,
                LPCTSTR lpName
            )->HANDLE {return (HANDLE)0x12;};
        ct().CloseHandle=[&] (HANDLE hObject)->BOOL {return TRUE;};
        ct().FindWindowW=
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
        ct().GetForegroundWindow=
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
                    FAIL("Don't pass here.");
                };
                return w;
            };
        ct().GetDoubleClickTime=
            [&] ()->UINT
            {
                h.calls().push_back(call("GetDoubleClickTime"));
                return 100;
            };
        ct().WaitForSingleObject=
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
        ct().canceled_event=make_shared<Event>();
        ct().properties.insert(make_pair("target","電卓"));
        wait_active();
        CHECK_EQUAL(6,h.calls().size());
        CHECK_EQUAL
        (
            call("FindWindowW",(const char*)NULL,(const char*)L"電卓"),
            h.calls().at(0)
        );
        CHECK_EQUAL(call("GetForegroundWindow"),h.calls().at(1));
        CHECK_EQUAL(call("GetDoubleClickTime"),h.calls().at(2));
        CHECK_EQUAL
        (call("WaitForSingleObject",(HANDLE)0x12,100),h.calls().at(3));
        CHECK_EQUAL
        (
            call("FindWindowW",(const char*)NULL,(const char*)L"電卓"),
            h.calls().at(4)
        );
        CHECK_EQUAL(call("GetForegroundWindow"),h.calls().at(5));
    });
}
