#ifndef ATAT_H
#define ATAT_H

#include<exception>
#include<functional>
#include<iostream>
#include<map>
#include<memory>
#include<sstream>
#include<stdexcept>
#include<string>
#include<vector>
#include<windows.h>

namespace atat
{
    using namespace std;

    struct canceled_exception:public exception {};

    class Row
    {
    public:
        Row(const string&description_);
        const string&description();
        const vector<string>&tokens();
    private:
        string description_;
        vector<string> tokens_;
    };

    class Command
    {
    public:
        Row*row();
        virtual void execute()=0;
    protected:
        Command(const shared_ptr<Row>&row_);
    private:
        shared_ptr<Row> row_;
    };

    class Event
    {
    public:
        Event();
        ~Event();
        const HANDLE&handle();
        void set();
    private:
        HANDLE handle_;
    };

    struct frame
    {
        size_t counter;
        size_t entry;
        size_t number;
    };

    struct context
    {
        void setup();

        function<BOOL(HANDLE)> CloseHandle;
        function<HANDLE(LPSECURITY_ATTRIBUTES,BOOL,BOOL,LPCTSTR)>
            CreateEvent;
        function<HWND(const wchar_t*,const wchar_t*)> FindWindowW;
        vector<frame> frames;
        function<UINT()> GetDoubleClickTime;
        function<HWND()> GetForegroundWindow;
        function<DWORD()> GetLastError;
        function<int(int)> GetSystemMetrics;
        function<BOOL(HWND,LPRECT)> GetWindowRect;
        function<UINT(UINT,LPINPUT,int)> SendInput;
        function<BOOL(PHANDLER_ROUTINE,BOOL)> SetConsoleCtrlHandler;
        function<BOOL(HANDLE)> SetEvent;
        function<DWORD(HANDLE,DWORD)> WaitForSingleObject;

        shared_ptr<Event> canceled_event;
        ostream*err;
        istream*in;
        size_t index;
        ostream*out;
        map<string,string> properties;
    };

    class KeyCommand:public Command
    {
    protected:
        KeyCommand(const shared_ptr<Row>&row_);
        static void send(const WORD&code,const DWORD&up);
        WORD code_;
    };

    class KeyDownCommand:public KeyCommand
    {
    public:
        KeyDownCommand(const shared_ptr<Row>&row_);
        virtual void execute() override;
    };

    class KeyPressCommand:public KeyCommand
    {
    public:
        KeyPressCommand(const shared_ptr<Row>&row_);
        virtual void execute() override;
    };

    class KeyUpCommand:public KeyCommand
    {
    public:
        KeyUpCommand(const shared_ptr<Row>&row_);
        virtual void execute() override;
    };

    class LoopBeginCommand:public Command
    {
    public:
        LoopBeginCommand(const shared_ptr<Row>&row_);
        virtual void execute() override;
    private:
        size_t number_;
    };

    class LoopEndCommand:public Command
    {
    public:
        LoopEndCommand(const shared_ptr<Row>&row_);
        virtual void execute() override;
    };

    class MouseCommand:public Command
    {
    protected:
        using Command::Command;
        static void send
        (
            const LONG&x,
            const LONG&y,
            const DWORD&amount,
            const DWORD&buttonAndAction
        );
    };

    class MouseButtonCommand:public MouseCommand
    {
    protected:
        MouseButtonCommand(const shared_ptr<Row>&row_);
        DWORD button_;
    };

    class MouseButtonClickCommand:public MouseButtonCommand
    {
    public:
        MouseButtonClickCommand(const shared_ptr<Row>&row_);
        virtual void execute() override;
    };

    class MouseButtonDoubleClickCommand:public MouseButtonCommand
    {
    public:
        MouseButtonDoubleClickCommand(const shared_ptr<Row>&row_);
        virtual void execute() override;
    };

    class MouseButtonDownCommand:public MouseButtonCommand
    {
    public:
        MouseButtonDownCommand(const shared_ptr<Row>&row_);
        virtual void execute() override;
    };

    class MouseButtonUpCommand:public MouseButtonCommand
    {
    public:
        MouseButtonUpCommand(const shared_ptr<Row>&row_);
        virtual void execute() override;
    };

    class MouseMoveCommand:public MouseCommand
    {
    public:
        MouseMoveCommand(const shared_ptr<Row>&row_);
        virtual void execute() override;
    private:
        LONG x_;
        LONG y_;
    };

    class MouseWheelCommand:public MouseCommand
    {
    public:
        MouseWheelCommand(const shared_ptr<Row>&row_);
        virtual void execute() override;
    private:
        LONG amount_;
    };

    class NullCommand:public Command
    {
    public:
        NullCommand(const shared_ptr<Row>&row_);
        virtual void execute() override;
    };

    class SleepCommand:public Command
    {
    public:
        SleepCommand(const shared_ptr<Row>&row_);
        virtual void execute() override;
    private:
        DWORD time_;
    };

    using COMMAND_FACTORY=
        function<shared_ptr<Command>(const shared_ptr<Row>&row)>;

    BOOL control_key_pressed(DWORD type);
    context&ct();
    template<class...ARGUMENTS> string describe(ARGUMENTS&&...arguments);
    template<class LEAD,class...TRAILER> void describe_to_with
    (
        ostream&os,
        const string&delimiter,
        LEAD&lead,
        TRAILER&&...trailer
    );
    template<class ARGUMENT> void describe_to_with
    (ostream&os,const string&delimiter,ARGUMENT&argument);
    inline void describe_to_with
    (ostream&os,const string&delimiter,const char*argument);
    inline string describe_to_with(ostream&os,const string&delimiter);
    template<class...ARGUMENTS> string describe_with
    (const string&delimiter,ARGUMENTS&&...arguments);
    int execute(int argc,char**argv);
    HWND find_target();
    void frame_begin(const size_t&number);
    bool frame_end();
    string lower_case(const string&source);
    shared_ptr<wchar_t> multi_to_wide
    (const string&str,const UINT&codePage);
    shared_ptr<Command> new_command
    (
        const map<string,COMMAND_FACTORY>&factories,
        const shared_ptr<Row>&row,
        const size_t&index
    );
    map<string,string> parse_properties(int argc,char**argv);
    vector<shared_ptr<Command>> parse_script();
    void run(const vector<shared_ptr<Command>>&commands);
    long to_number(const string&str);
    vector<string> tokenize(const string&str,const string&delimiters);
    void wait(const DWORD&time);
    void wait_active();

    template<class... ARGUMENTS> string describe(ARGUMENTS&&...arguments)
    {return describe_with("",arguments...);}

    template<class LEAD,class... TRAILER> void describe_to_with
    (
        ostream&os,
        const string&delimiter,
        LEAD&lead,
        TRAILER&&...trailer
    )
    {
        describe_to_with(os,delimiter,lead);
        os<<delimiter;
        describe_to_with(os,delimiter,trailer...);
    }

    template<class ARGUMENT> void describe_to_with
    (ostream&os,const string&delimiter,ARGUMENT&argument) {os<<argument;}

    inline void describe_to_with
    (ostream&os,const string&delimiter,const char*argument)
    {if(argument!=nullptr) os<<argument;}

    inline string describe_to_with(ostream&os,const string&delimiter)
    {return "";}

    template<class...ARGUMENTS> string describe_with
    (const string&delimiter,ARGUMENTS&&...arguments)
    {
        ostringstream os;
        describe_to_with(os,delimiter,arguments...);
        return os.str();
    }
}

#endif
