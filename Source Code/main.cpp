
#include <SFML/Network.hpp>
#include <iostream>
#include <chrono>
#include <thread>

using namespace std;
using namespace sf;

// Enviar evento ao servidor
bool sendEvent(string str, TcpSocket *socket) {

	Packet packet_send;

	packet_send << str;

	// Verificar se o socket enviou o pacote com sucesso
	if (socket->send(packet_send) != Socket::Done) {
		packet_send.clear();
		return false;
	}

	cout << str << endl;

	packet_send.clear();

	return true;

}

// Enviar evento de status que contem yes ou no na str se o numero e certo ou errado e o numero restante de numeros
bool sendEventStatus(string str, int leftValues, TcpSocket *socket) {

	Packet packet_send;

	cout << " Match: " << str << endl;

	packet_send << str << leftValues;

	// Verificar se o socket enviou o pacote com sucesso
	if (socket->send(packet_send) != Socket::Done) {
		packet_send.clear();
		return false;
	}

	packet_send.clear();

	return true;

}

// Receber evento do servidor
string receiveEvent(TcpSocket *socket) {

	Packet packet_receive;
	string str = "";

	// Verifica se o socket recebeu o pacote com sucesso
	if (socket->receive(packet_receive) != Socket::Done) {
		packet_receive.clear();
		return "";
	}

	packet_receive >> str;

	packet_receive.clear();

	return str;

}

// Receber numero do servidor
int receiveNumber(TcpSocket *socket) {

	Packet packet_receive;
	int numero = 0;

	// Verifica se o socket recebeu o pacote com sucesso
	if (socket->receive(packet_receive) != Socket::Done) {
		packet_receive.clear();
		return -1;
	}

	packet_receive >> numero;
	cout << " Number: " << numero << endl;

	packet_receive.clear();

	return numero;

}

// Receber o cartao de numeros do servidor
vector<int> receiveCard(TcpSocket *socket) {

	Packet packet_receive;
	string err = "n";

	vector<int> card(0);

	// Verifica se o socket recebeu o pacote com sucesso
	if (socket->receive(packet_receive) != Socket::Done) {
		packet_receive.clear();
		err = "y";
	}

	// Preencher o vector de numeros se nao houver erro no recebimento do pacote
	if (err.compare("n") == 0) {

		for (int i = 0; i < 10; i++) {

			int num = 0;
			packet_receive >> num;
			card.push_back(num);

		}

	}

	packet_receive.clear();

	return card;

}

int main() {

	TcpSocket socket;
	IpAddress ip = IpAddress::getLocalAddress();
	Packet packet_receive;
	vector<int> card(0);

	socket.connect(ip, 2000);

	cout << ".:: Jogador Online - Made by Telmo Reinas for Fabamaq ::. \n" << endl;

	// Recebe evento Connected e envia evento Jogo
	if (receiveEvent(&socket).compare("Connected") != 0) {
		cout << "Error receiving event Coonected \n" << endl;
		socket.disconnect();
	}

	if (!sendEvent("Jogo", &socket)) {
		cout << "Error sending Event Jogo \n" << endl;
		socket.disconnect();
	}

	// Receber o cartao de numeros do servidor
	card = receiveCard(&socket);

	// Enviar confirmacao da rececao do cartao
	if (!sendEvent("Received Card", &socket)) {
		cout << "Error sending Event Received Card \n" << endl;
		socket.disconnect();
	}

	vector<bool> cardAux(10);
	int totalNumbers = 0;

	// Preencher um vector de booleans a false, quando os valores fizerem match, a respectiva posicao sera true
	for (int aux = 0; aux < 10; aux++) {
		cardAux[aux] = false;
	}

	// Correr ate todos os numeros recebidos do servidor fizerem match com os numeros do cartao
	while (true) {

		int number = receiveNumber(&socket);
		int size = card.size();

		// Flag existe tera o valor de yes se houver match e altera o evento enviado ao servidor
		string existe = "no";

		// Percorrer o cartao e fazer o match do numero recebido com o vector de numeros do jogador
		for (int i = 0; i < size; i++) {

			cout << " Total Matches: " << totalNumbers << endl;

			if (number == card[i] && cardAux[i] == false) {

				cardAux[i] = true;
				existe = "yes";
				totalNumbers += 1;
				break;

			}

		}

		for (int k = 0; k < 10; k++) {
			cout << " Card number: " << card[k] << " Matched Number: " << cardAux[k] ? "true" : "false";
			cout << endl;
		}

		// Total de numeros restantes e envio do evento de match ao servidor
		int leftValues = 10 - totalNumbers;

		if (existe.compare("yes") == 0)
			sendEventStatus("yes", leftValues, &socket);
		else
			sendEventStatus("No", leftValues, &socket);

		existe = "no";

		// Terminar o ciclo e jogo quando todos os numeros fizerem match
		if (totalNumbers == 10)
			break;

	}

	// Enviar evento won ao servidor
	if (totalNumbers == 10) {
		sendEvent("Won", &socket);
	}

	return 0;

}

