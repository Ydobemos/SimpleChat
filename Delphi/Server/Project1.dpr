program ChatServer;

{$APPTYPE CONSOLE}

uses
  SysUtils,
  Classes,
  IdTCPServer,
  IdContext;

type
  TChatServer = class
  private
    FServer: TIdTCPServer;
    FSessions: TList;
    procedure HandleExecute(AContext: TIdContext);
    procedure BroadcastMessage(const AMsg: string; ASender: TIdContext);
  public
    constructor Create(APort: Integer);
    destructor Destroy; override;
    procedure Start;
  end;

constructor TChatServer.Create(APort: Integer);
begin
  FServer := TIdTCPServer.Create(nil);
  FServer.DefaultPort := APort;
  FServer.OnExecute := HandleExecute;
  FSessions := TList.Create;
end;

destructor TChatServer.Destroy;
begin
  FServer.Free;
  FSessions.Free;
  inherited;
end;

procedure TChatServer.HandleExecute(AContext: TIdContext);
var
  LMsg: string;
begin
  FSessions.Add(AContext);
  try
    while AContext.Connection.Connected do
    begin
      LMsg := AContext.Connection.IOHandler.ReadLn;
      BroadcastMessage(LMsg, AContext);
    end;
  finally
    FSessions.Remove(AContext);
  end;
end;

procedure TChatServer.BroadcastMessage(const AMsg: string; ASender: TIdContext);
var
  I: Integer;
  Context: TIdContext;
begin
  for I := 0 to FSessions.Count - 1 do
  begin
    Context := TIdContext(FSessions[I]);
    if Context <> ASender then
    begin
      Context.Connection.IOHandler.WriteLn(AMsg);
    end;
  end;
end;

procedure TChatServer.Start;
begin
  FServer.Active := True;
  WriteLn('Chat server started...');
end;

var
  MyChatServer: TChatServer;
begin
  if ParamCount <> 1 then
  begin
    WriteLn('Usage: ChatServer <port>');
    Exit;
  end;

  try
    MyChatServer := TChatServer.Create(StrToInt(ParamStr(1)));
    try
      MyChatServer.Start;
      ReadLn; // Keep the console open
    finally
      MyChatServer.Free;
    end;
  except
    on E: Exception do
      Writeln(E.ClassName, ': ', E.Message);
  end;
end.

