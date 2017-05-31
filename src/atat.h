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

    struct aborted_exception;
    class Context;
    struct frame;
    class KeyCommand;
    class KeyDownCommand;
    class KeyPressCommand;
    class KeyUpCommand;
    class LoopBeginCommand;
    class LoopEndCommand;
    class MouseButtonClickCommand;
    class MouseButtonDoubleClickCommand;
    class MouseButtonDownCommand;
    class MouseButtonCommand;
    class MouseButtonUpCommand;
    class MouseCommand;
    class MouseMoveCommand;
    class MouseWheelCommand;
    class NullCommand;
    class Row;
    class SleepCommand;
    class SystemObject;

    struct aborted_exception:public exception {};

    class Context
    {
    public:
        Context();
        const HANDLE&abortedEvent();
        vector<frame>&frames();
        size_t&index();
        static shared_ptr<Context>&instance();
        map<string,string>&properties();
    private:
        shared_ptr<SystemObject> abortedEvent_;
        vector<frame> frames_;
        size_t index_;
        static shared_ptr<Context> instance_;
        map<string,string> properties_;
    };

    struct frame
    {
        size_t counter;
        size_t entry;
        size_t number;
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

    class SystemObject
    {
    public:
        SystemObject(HANDLE handle_);
        ~SystemObject();
        const HANDLE&handle();
    private:
        HANDLE handle_;
    };

    using COMMAND_FACTORY=
        function<shared_ptr<Command>(const shared_ptr<Row>&row)>;

    BOOL control_key_pressed(DWORD type);
    template<class...ARGUMENTS> string describe(ARGUMENTS&&...arguments);
    template<class LEAD,class...TRAILER> void describe_to_with
    (
        ostream&os,
        const string&delimiter,
        const LEAD&lead,
        TRAILER&&...trailer
    );
    template<class ARGUMENT> void describe_to_with
    (ostream&os,const string&delimiter,const ARGUMENT&argument);
    inline string describe_to_with(ostream&os,const string&delimiter);
    template<class...ARGUMENTS> string describe_with
    (const string&delimiter,ARGUMENTS&&...arguments);
    HWND find_target();
    string lower_case(const string&source);
    shared_ptr<wchar_t> multi_to_wide
    (const string&str,const UINT&codePage);
    shared_ptr<Command> new_command
    (
        const map<string,COMMAND_FACTORY>&commandFactories,
        const shared_ptr<Row>&row,
        const size_t&switchIndex
    );
    map<string,string> parse_properties(int argc,char**argv);
    vector<shared_ptr<Command>> parse_script(istream&is);
    map<string,string>&properties();
    int run(int argc,char**argv,istream&in,ostream&out,ostream&err);
    void setup_io();
    vector<string> tokenize(const string&str,const string&delimiters);
    void wait(const DWORD&time);
    void wait_active();

    extern function<BOOL(HANDLE)> CloseHandle;
    extern function<HANDLE(LPSECURITY_ATTRIBUTES,BOOL,BOOL,LPCTSTR)>
        CreateEvent;
    extern function<HWND(const wchar_t*,const wchar_t*)> FindWindowW;
    extern function<UINT()> GetDoubleClickTime;
    extern function<HWND()> GetForegroundWindow;
    extern function<DWORD()> GetLastError;
    extern function<int(int)> GetSystemMetrics;
    extern function<BOOL(HWND,LPRECT)> GetWindowRect;
    extern function<UINT(UINT,LPINPUT,int)> SendInput;
    extern function<BOOL(PHANDLER_ROUTINE,BOOL)>
        SetConsoleCtrlHandler;
    extern function<BOOL(HANDLE)> SetEvent;
    extern function<DWORD(HANDLE,DWORD)> WaitForSingleObject;

    template<class... ARGUMENTS> string describe(ARGUMENTS&&...arguments)
    {return describe_with("",arguments...);}

    template<class LEAD,class... TRAILER> void describe_to_with
    (
        ostream &os,
        const string&delimiter,
        const LEAD &lead,
        TRAILER&&...trailer
    )
    {
        describe_to_with(os,delimiter,lead);
        os<<delimiter;
        describe_to_with(os,delimiter,trailer...);
    }

    template<class ARGUMENT> void describe_to_with
    (ostream &os,const string&delimiter,const ARGUMENT &argument)
    {os<<argument;}

    inline string describe_to_with(ostream &os,const string&delimiter)
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
