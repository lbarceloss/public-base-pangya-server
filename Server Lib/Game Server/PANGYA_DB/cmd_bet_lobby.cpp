
#if defined(_WIN32)
#pragma pack(1)
#endif

#include "cmd_bet_lobby.hpp"

#include <cstring>

using namespace stdA;

CmdBetLobbyPendente::CmdBetLobbyPendente(bool _waiter) : pangya_db(_waiter) {
}

CmdBetLobbyPendente::~CmdBetLobbyPendente() {
}

void CmdBetLobbyPendente::lineResult(result_set::ctx_res* _result, uint32_t  ) {

	checkColumnNumber(6, (uint32_t)_result->cols);

	ctx_bet_pendente ctx_b{ 0 };

	ctx_b.id           = (uint32_t)IFNULL(atoi, _result->data[0]);
	ctx_b.criador_uid  = (uint32_t)IFNULL(atoi, _result->data[1]);
	ctx_b.oponente_uid = (uint32_t)IFNULL(atoi, _result->data[2]);
	ctx_b.course       = (unsigned char)IFNULL(atoi, _result->data[3]);
	ctx_b.holes        = (unsigned char)IFNULL(atoi, _result->data[4]);

	memset(ctx_b.pin, 0, sizeof(ctx_b.pin));

	if (_result->data[5] != nullptr) {
		size_t n = strlen(_result->data[5]);
		if (n > sizeof(ctx_b.pin) - 1)
			n = sizeof(ctx_b.pin) - 1;
		memcpy(ctx_b.pin, _result->data[5], n);
	}

	v_bet.push_back(ctx_b);
}

response* CmdBetLobbyPendente::prepareConsulta(database& _db) {

	if (!v_bet.empty())
		v_bet.clear();

	auto r = consulta(_db, m_szConsulta);

	checkResponse(r, "nao conseguiu ler as apostas pendentes (bet_lobby)");

	return r;
}

std::vector< ctx_bet_pendente >& CmdBetLobbyPendente::getInfo() {
	return v_bet;
}

CmdBetLobbySalaCriada::CmdBetLobbySalaCriada(uint32_t _id, short _sala_numero, bool _waiter)
	: pangya_db(_waiter), m_id(_id), m_sala_numero(_sala_numero) {
}

CmdBetLobbySalaCriada::~CmdBetLobbySalaCriada() {
}

void CmdBetLobbySalaCriada::lineResult(result_set::ctx_res*  , uint32_t  ) {

}

response* CmdBetLobbySalaCriada::prepareConsulta(database& _db) {

	std::string sql = "UPDATE pangya.bet_lobby SET estado = 'sala_criada', sala_numero = "
			+ std::to_string((int)m_sala_numero)
			+ " WHERE id = " + std::to_string(m_id) + " AND estado = 'aceita'";

	auto r = consulta(_db, sql.c_str());

	checkResponse(r, "nao conseguiu marcar a aposta[id=" + std::to_string(m_id) + "] como sala_criada");

	return r;
}

CmdBetLobbyComSala::CmdBetLobbyComSala(bool _waiter) : pangya_db(_waiter) {
}

CmdBetLobbyComSala::~CmdBetLobbyComSala() {
}

void CmdBetLobbyComSala::lineResult(result_set::ctx_res* _result, uint32_t  ) {

	checkColumnNumber(2, (uint32_t)_result->cols);

	ctx_bet_com_sala ctx_b{ 0 };

	ctx_b.id          = (uint32_t)IFNULL(atoi, _result->data[0]);
	ctx_b.sala_numero = (short)IFNULL(atoi, _result->data[1]);

	v_bet.push_back(ctx_b);
}

response* CmdBetLobbyComSala::prepareConsulta(database& _db) {

	if (!v_bet.empty())
		v_bet.clear();

	auto r = consulta(_db, m_szConsulta);

	checkResponse(r, "nao conseguiu ler as apostas com sala (bet_lobby)");

	return r;
}

std::vector< ctx_bet_com_sala >& CmdBetLobbyComSala::getInfo() {
	return v_bet;
}

CmdBetLobbyEncerrar::CmdBetLobbyEncerrar(uint32_t _id, bool _waiter)
	: pangya_db(_waiter), m_id(_id) {
}

CmdBetLobbyEncerrar::~CmdBetLobbyEncerrar() {
}

void CmdBetLobbyEncerrar::lineResult(result_set::ctx_res*  , uint32_t  ) {
}

response* CmdBetLobbyEncerrar::prepareConsulta(database& _db) {

	std::string sql = "UPDATE pangya.bet_lobby SET estado = 'encerrada' WHERE id = "
			+ std::to_string(m_id) + " AND estado = 'sala_criada'";

	auto r = consulta(_db, sql.c_str());

	checkResponse(r, "nao conseguiu encerrar a aposta[id=" + std::to_string(m_id) + "]");

	return r;
}
