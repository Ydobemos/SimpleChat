// chat_server.cpp

// Einbinden der notwendigen Bibliotheken
#include <iostream>             // Für die Ein- und Ausgabe
#include <set>                  // Für die Verwaltung der verbundenen Clients
#include <memory>               // Für shared_ptr
#include <boost/asio.hpp>       // Boost.Asio Bibliothek für Netzwerkprogrammierung

// Namespace für einfacheren Zugriff auf Boost.Asio Komponenten
using boost::asio::ip::tcp;
using namespace std;

// Klasse, die eine Sitzung für jeden verbundenen Client repräsentiert
class ChatSession : public enable_shared_from_this<ChatSession> {
public:
	// Konstruktor, der einen Socket und eine Referenz auf die Menge der Sitzungen erhält
	ChatSession(tcp::socket socket, set<shared_ptr<ChatSession>>& sessions)
		: socket_(move(socket)), sessions_(sessions) {}

	// Methode zum Starten der Sitzung
	void start() {
		sessions_.insert(shared_from_this());  // Sitzung zur Menge hinzufügen
		do_read();                             // Leseoperation starten
	}

	// Methode zum Senden einer Nachricht an diesen Client
	void deliver(const string& msg) {
		auto self(shared_from_this());         // shared_ptr für diese Sitzung erstellen
		boost::asio::async_write(socket_, boost::asio::buffer(msg), [this, self](boost::system::error_code ec, size_t) {
			if (ec) {                          // Wenn ein Fehler auftritt
				sessions_.erase(shared_from_this());  // Sitzung aus der Menge entfernen
			}
			});
	}

private:
	// Methode zum Asynchronen Lesen von Daten
	void do_read() {
		auto self(shared_from_this());         // shared_ptr für diese Sitzung erstellen
		boost::asio::async_read_until(socket_, boost::asio::dynamic_buffer(data_), "\n", [this, self](boost::system::error_code ec, size_t length) {
			if (!ec) {                         // Wenn kein Fehler auftritt
				string msg = data_.substr(0, length); // Empfangene Nachricht extrahieren
				data_.erase(0, length);        // Empfangene Daten aus dem Puffer entfernen
				// Nachricht an alle anderen Sitzungen weiterleiten
				for (auto& session : sessions_) {
					if (session != shared_from_this()) {
						session->deliver(msg);
					}
				}
				do_read();                     // Wiederholtes Lesen starten
			}
			else {
				sessions_.erase(shared_from_this());  // Bei Fehler Sitzung entfernen
			}
			});
	}

	tcp::socket socket_;                      // Socket für die Kommunikation
	string data_;                             // Puffer für empfangene Daten
	set<shared_ptr<ChatSession>>& sessions_;  // Referenz auf die Menge der Sitzungen
};

// Klasse, die den Chat-Server repräsentiert
class ChatServer {
public:
	// Konstruktor, der den io_context und Endpunkt für den Server erhält
	ChatServer(boost::asio::io_context& io_context, const tcp::endpoint& endpoint)
		: acceptor_(io_context, endpoint) {
		do_accept();  // Startet den Asynchronen Accept-Betrieb
	}

private:
	// Methode zum Asynchronen Akzeptieren neuer Verbindungen
	void do_accept() {
		acceptor_.async_accept([this](boost::system::error_code ec, tcp::socket socket) {
			if (!ec) {  // Wenn kein Fehler auftritt
				make_shared<ChatSession>(move(socket), sessions_)->start(); // Neue Sitzung starten
			}
			do_accept();  // Wiederholtes Akzeptieren starten
			});
	}

	tcp::acceptor acceptor_;                  // Acceptor für eingehende Verbindungen
	set<shared_ptr<ChatSession>> sessions_;   // Menge der aktiven Sitzungen
};

// Hauptfunktion
int main(int argc, char* argv[]) {
	try {
		if (argc != 2) {  // Überprüfen, ob die Portnummer angegeben wurde
			cerr << "Usage: chat_server <port>\n";
			return 1;  // Programm mit Fehlercode beenden
		}

		boost::asio::io_context io_context;  // Erstellen des io_context
		tcp::endpoint endpoint(tcp::v4(), stoi(argv[1]));  // Erstellen des Endpunkts mit der angegebenen Portnummer
		ChatServer server(io_context, endpoint);  // Erstellen des Chat-Servers
		io_context.run();  // Starten des io_context, um Asynchrone Operationen auszuführen
	}
	catch (exception& e) {
		cerr << "Exception: " << e.what() << "\n";  // Ausnahmebehandlung und Fehlerausgabe
	}

	return 0;  // Erfolgreiches Beenden des Programms
}