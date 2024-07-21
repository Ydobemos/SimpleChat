program ChatClient;

{$APPTYPE CONSOLE}

uses
  SysUtils,
  Classes,
  IdTCPClient,
  IdGlobal;

type
  TChatClient = class
  private
    FClient: TIdTCPClient;
    procedure ReceiveMessages;
  public
    constructor Create(AHost: string; APort: Integer);
    destructor Destroy; override;
    procedure Start;
  end;

constructor TChatClient.Create(AHost: string; APort: Integer);
begin
  FClient := TIdTCPClient.Create(nil);
  FClient.Host := AHost;
  FClient.Port := APort;
end;

destructor TChatClient.Destroy;
begin
  FClient.Free;
  inherited;
end;

procedure TChatClient.ReceiveMessages;
var
  LMsg: string;
begin
  while FClient.Connected do
  begin
    LMsg := FClient.IOHandler.ReadLn;
    WriteLn(LMsg);
  end;
end;

procedure TChatClient.Start;
var
  Line: string;
begin
  FClient.ConnectTimeout := 5000;
  FClient.Connect;
  try
    TThread.CreateAnonymousThread(ReceiveMessages).Start;
    while True do
    begin
      ReadLn(Line);
      FClient.IOHandler.WriteLn(Line);
    end;
  finally
    FClient.Disconnect;
  end;
end;

var
  MyChatClient: TChatClient;
begin
  if ParamCount <> 2 then
  begin
    WriteLn('Usage: ChatClient <host> <port>');
    Exit;
  end;

  try
    MyChatClient := TChatClient.Create(ParamStr(1), StrToInt(ParamStr(2)));
    try
      MyChatClient.Start;
    finally
      MyChatClient.Free;
    end;
  except
    on E: Exception do
      Writeln(E.ClassName, ': ', E.Message);
  end;
end.

