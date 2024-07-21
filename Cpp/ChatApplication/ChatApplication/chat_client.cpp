// chat_client.cpp

// Einbinden der notwendigen Bibliotheken
#include <iostream>             // Für die Ein- und Ausgabe
#include <thread>               // Für die Verwaltung von Threads
#include <deque>                // Für die Verwendung von deque
#include <string>               // Für die Verwendung von string
#include <boost/asio.hpp>       // Boost.Asio Bibliothek für Netzwerkprogrammierung

// Namespace für einfacheren Zugriff auf Boost.Asio Komponenten
using boost::asio::ip::tcp;
using namespace std;

// Klasse, die den Chat-Client repräsentiert
class ChatClient {
public:
	// Konstruktor, der den io_context und die Endpunkte für die Verbindung erhält
	ChatClient(boost::asio::io_context& io_context, const tcp::resolver::results_type& endpoints)
		: io_context_(io_context), socket_(io_context) {
		do_connect(endpoints);  // Startet den Verbindungsaufbau
	}

	// Methode zum Senden einer Nachricht
	void write(const string& msg) {
		boost::asio::post(io_context_, [this, msg]() {
			auto write_in_progress = !write_msgs_.empty();  // Überprüfen, ob eine Nachricht gesendet wird
			write_msgs_.push_back(msg);  // Nachricht zur Warteschlange hinzufügen
			if (!write_in_progress) {    // Wenn keine Nachricht gesendet wird
				do_write();  // Sendeoperation starten
			}
			});
	}

	// Methode zum Schließen der Verbindung
	void close() {
		boost::asio::post(io_context_, [this]() { socket_.close(); });
	}

private:
	// Methode zum Asynchronen Verbindungsaufbau
	void do_connect(const tcp::resolver::results_type& endpoints) {
		boost::asio::async_connect(socket_, endpoints, [this](boost::system::error_code ec, tcp::endpoint) {
			if (!ec) {  // Wenn kein Fehler auftritt
				do_read();  // Leseoperation starten
			}
			});
	}

	// Methode zum Asynchronen Lesen von Daten
	void do_read() {
		boost::asio::async_read_until(socket_, boost::asio::dynamic_buffer(read_msg_), "\n", [this](boost::system::error_code ec, size_t length) {
			if (!ec) {  // Wenn kein Fehler auftritt
				cout.write(read_msg_.data(), length);  // Empfangene Nachricht anzeigen
				cout.flush();  // Ausgabepuffer leeren
				read_msg_.erase(0, length);  // Empfangene Daten aus dem Puffer entfernen
				do_read();  // Wiederholtes Lesen starten
			}
			else {
				socket_.close();  // Bei Fehler Verbindung schließen
			}
			});
	}

	// Methode zum Asynchronen Senden von Daten
	void do_write() {
		boost::asio::async_write(socket_, boost::asio::buffer(write_msgs_.front()), [this](boost::system::error_code ec, size_t) {
			if (!ec) {  // Wenn kein Fehler auftritt
				write_msgs_.pop_front();  // Gesendete Nachricht aus der Warteschlange entfernen
				if (!write_msgs_.empty()) {  // Wenn noch Nachrichten in der Warteschlange sind
					do_write();  // Wiederholtes Senden starten
				}
			}
			else {
				socket_.close();  // Bei Fehler Verbindung schließen
			}
			});
	}

	boost::asio::io_context& io_context_;  // Referenz auf den io_context
	tcp::socket socket_;  // Socket für die Kommunikation
	string read_msg_;  // Puffer für empfangene Daten
	deque<string> write_msgs_;  // Warteschlange für zu sendende Nachrichten
};

// Hauptfunktion
int main(int argc, char* argv[]) {
	try {
		if (argc != 3) {  // Überprüfen, ob Host und Port angegeben wurden
			cerr << "Usage: chat_client <host> <port>\n";
			return 1;  // Programm mit Fehlercode beenden
		}

		boost::asio::io_context io_context;  // Erstellen des io_context

		tcp::resolver resolver(io_context);  // Erstellen des Resolvers für die Namensauflösung
		auto endpoints = resolver.resolve(argv[1], argv[2]);  // Auflösen des Hosts und Ports
		ChatClient c(io_context, endpoints);  // Erstellen des Chat-Clients

		thread t([&io_context]() { io_context.run(); });  // Starten eines Threads für den io_context

		string line;
		while (getline(cin, line)) {  // Lesen von Benutzereingaben aus der Konsole
			c.write(line + "\n");  // Senden der Nachricht an den Server
		}

		c.close();  // Verbindung schließen
		t.join();  // Warten, bis der io_context-Thread beendet ist
	}
	catch (exception& e) {
		cerr << "Exception: " << e.what() << "\n";  // Ausnahmebehandlung und Fehlerausgabe
	}

	return 0;  // Erfolgreiches Beenden des Programms
}