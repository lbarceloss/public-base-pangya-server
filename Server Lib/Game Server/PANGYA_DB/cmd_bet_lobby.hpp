
#pragma once
#ifndef _STDA_CMD_BET_LOBBY_HPP
#define _STDA_CMD_BET_LOBBY_HPP

#include "../../Projeto IOCP/PANGYA_DB/pangya_db.h"

#include <string>
#include <vector>

namespace stdA {

	struct ctx_bet_pendente {
		uint32_t id;
		uint32_t criador_uid;
		uint32_t oponente_uid;
		unsigned char course;
		unsigned char holes;
		char pin[8];
	};

	class CmdBetLobbyPendente : public pangya_db {
		public:
			explicit CmdBetLobbyPendente(bool _waiter = false);
			virtual ~CmdBetLobbyPendente();

			std::vector< ctx_bet_pendente >& getInfo();

		protected:
			void lineResult(result_set::ctx_res* _result, uint32_t _index_result) override;
			response* prepareConsulta(database& _db) override;

			std::string _getName() override { return "CmdBetLobbyPendente"; };
			std::wstring _wgetName() override { return L"CmdBetLobbyPendente"; };

		private:
			std::vector< ctx_bet_pendente > v_bet;

			const char* m_szConsulta =
				"SELECT id, criador_uid, oponente_uid, course, holes, pin "
				"FROM pangya.bet_lobby WHERE estado = 'aceita' ORDER BY id";
	};

	struct ctx_bet_com_sala {
		uint32_t id;
		short    sala_numero;
	};

	class CmdBetLobbyComSala : public pangya_db {
		public:
			explicit CmdBetLobbyComSala(bool _waiter = false);
			virtual ~CmdBetLobbyComSala();

			std::vector< ctx_bet_com_sala >& getInfo();

		protected:
			void lineResult(result_set::ctx_res* _result, uint32_t _index_result) override;
			response* prepareConsulta(database& _db) override;

			std::string _getName() override { return "CmdBetLobbyComSala"; };
			std::wstring _wgetName() override { return L"CmdBetLobbyComSala"; };

		private:
			std::vector< ctx_bet_com_sala > v_bet;

			const char* m_szConsulta =
				"SELECT id, sala_numero FROM pangya.bet_lobby "
				"WHERE estado = 'sala_criada' AND sala_numero IS NOT NULL ORDER BY id";
	};

	class CmdBetLobbyEncerrar : public pangya_db {
		public:
			CmdBetLobbyEncerrar(uint32_t _id, bool _waiter = false);
			virtual ~CmdBetLobbyEncerrar();

		protected:
			void lineResult(result_set::ctx_res* _result, uint32_t _index_result) override;
			response* prepareConsulta(database& _db) override;

			std::string _getName() override { return "CmdBetLobbyEncerrar"; };
			std::wstring _wgetName() override { return L"CmdBetLobbyEncerrar"; };

		private:
			uint32_t m_id;
	};

	class CmdBetLobbySalaCriada : public pangya_db {
		public:
			CmdBetLobbySalaCriada(uint32_t _id, short _sala_numero, bool _waiter = false);
			virtual ~CmdBetLobbySalaCriada();

		protected:
			void lineResult(result_set::ctx_res* _result, uint32_t _index_result) override;
			response* prepareConsulta(database& _db) override;

			std::string _getName() override { return "CmdBetLobbySalaCriada"; };
			std::wstring _wgetName() override { return L"CmdBetLobbySalaCriada"; };

		private:
			uint32_t m_id;
			short    m_sala_numero;
	};
}

#endif
