
#if defined(_WIN32)
#pragma pack(1)
#endif

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <windows.h>

#include "SOCKET/session.h"
#include "THREAD POOL/threadpool_base.hpp"
#include "UTIL/exception.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <deque>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>

#pragma comment(lib, "ws2_32.lib")

using namespace stdA;

static int g_fail = 0;
#define CHECK(cond, msg) do { if (!(cond)) { printf("  [FALHOU] %s\n", (msg)); ++g_fail; } } while (0)

struct posted_op {
	session* s;
	Buffer*  b;
	DWORD    op;
};

class mock_pool : public threadpool_base {
public:
	void postIoOperation(session* _session, Buffer* lpBuffer, DWORD dwIOsize, DWORD operation) override {
		(void)dwIOsize;
		std::lock_guard<std::mutex> lk(m_mtx);
		m_q.push_back(posted_op{ _session, lpBuffer, operation });
	}
	bool pop(posted_op& out) {
		std::lock_guard<std::mutex> lk(m_mtx);
		if (m_q.empty()) return false;
		out = m_q.front();
		m_q.pop_front();
		return true;
	}
	size_t size() { std::lock_guard<std::mutex> lk(m_mtx); return m_q.size(); }
private:
	std::mutex m_mtx;
	std::deque<posted_op> m_q;
};

class test_session : public session {
public:
	test_session(threadpool_base& tp, SOCKET s, SOCKADDR_IN a, unsigned char k) : session(tp, s, a, k) {}
	unsigned char getStateLogged() override { return 1u; }
	uint32_t getUID() override { return 4242u; }
	uint32_t getCapability() override { return 0u; }
	char* getNickname() override { static char n[] = "teste"; return n; }
	char* getID() override { static char i[] = "teste"; return i; }
};

static bool make_pair(SOCKET& srv_side, SOCKET& cli_side) {
	SOCKET lst = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (lst == INVALID_SOCKET) return false;

	SOCKADDR_IN a; memset(&a, 0, sizeof(a));
	a.sin_family = AF_INET;
	a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	a.sin_port = 0;
	if (::bind(lst, (SOCKADDR*)&a, sizeof(a)) != 0) return false;
	int alen = sizeof(a);
	if (::getsockname(lst, (SOCKADDR*)&a, &alen) != 0) return false;
	if (::listen(lst, 1) != 0) return false;

	cli_side = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (cli_side == INVALID_SOCKET) return false;
	if (::connect(cli_side, (SOCKADDR*)&a, sizeof(a)) != 0) return false;

	srv_side = ::accept(lst, nullptr, nullptr);
	::closesocket(lst);
	return srv_side != INVALID_SOCKET;
}

struct io_sim {
	std::vector<unsigned char> stream;
	std::vector<size_t>        lot_size;
	std::vector<bool>          lot_raw;

	bool run_one(const posted_op& p) {
		bool raw = (p.op == STDA_OT_SEND_RAW_REQUEST);
		try {
			p.s->setSend();
		} catch (exception&) {
			return false;
		}
		WSABUF* wb = p.b->getWSABufToSend();
		size_t len = (size_t)wb->len;
		if (len > 0) {
			stream.insert(stream.end(), (unsigned char*)wb->buf, (unsigned char*)wb->buf + len);
			lot_size.push_back(len);
			lot_raw.push_back(raw);
			p.b->consume(len);
		}
		p.s->releaseSend();
		return true;
	}
};

static void teste_ordem_simples(mock_pool& pool, test_session& s) {
	printf("[1] ordem com produtor unico + drenagem intercalada\n");

	io_sim io;
	std::vector<unsigned char> esperado;

	const size_t tam[] = { 10, 1, 4000, 16384, 20000, 7, 33000, 512, 65000, 3 };
	unsigned char seed = 0;

	for (size_t i = 0; i < sizeof(tam) / sizeof(tam[0]); ++i) {
		std::vector<unsigned char> pkt(tam[i]);
		for (size_t j = 0; j < pkt.size(); ++j) pkt[j] = seed++;
		esperado.insert(esperado.end(), pkt.begin(), pkt.end());

		s.requestSendBuffer(pkt.data(), pkt.size());

		if (i % 3 == 0) {
			posted_op p;
			while (pool.pop(p)) io.run_one(p);
		}
	}

	posted_op p;
	while (pool.pop(p)) io.run_one(p);

	CHECK(io.stream.size() == esperado.size(), "tamanho total do stream diferente do enviado");
	CHECK(io.stream == esperado, "ORDEM/CONTEUDO do stream divergiu do que foi pedido");
	printf("      %zu bytes em %zu lotes (WSASend)\n", io.stream.size(), io.lot_size.size());
}

static void teste_raw_nao_mistura(mock_pool& pool, test_session& s) {
	printf("[2] raw x nao-raw nunca no mesmo WSASend\n");

	io_sim io;
	std::vector<unsigned char> esperado;
	std::vector<bool>          esperado_raw;

	for (int i = 0; i < 40; ++i) {
		bool raw = (i % 2) == 0;
		size_t n = 100 + (i * 37) % 900;
		std::vector<unsigned char> pkt(n, (unsigned char)(raw ? 0xAA : 0x55));
		esperado.insert(esperado.end(), pkt.begin(), pkt.end());
		esperado_raw.insert(esperado_raw.end(), n, raw);

		s.requestSendBuffer(pkt.data(), pkt.size(), raw);

		if (i % 5 == 0) { posted_op p; while (pool.pop(p)) io.run_one(p); }
	}
	posted_op p;
	while (pool.pop(p)) io.run_one(p);

	CHECK(io.stream == esperado, "ORDEM/CONTEUDO divergiu no teste raw");

	size_t off = 0;
	bool misturou = false;
	for (size_t l = 0; l < io.lot_size.size(); ++l) {
		for (size_t k = 0; k < io.lot_size[l]; ++k) {
			if (off + k < esperado_raw.size() && esperado_raw[off + k] != io.lot_raw[l]) {
				misturou = true;
			}
		}
		off += io.lot_size[l];
	}
	CHECK(!misturou, "um WSASend MISTUROU bytes raw com nao-raw");
	printf("      %zu bytes em %zu lotes, nenhum lote misturado\n", io.stream.size(), io.lot_size.size());
}

static void teste_nao_bloqueia(mock_pool& pool, test_session& s) {
	printf("[3] nao bloqueia com o socket parado (o bug original)\n");

	unsigned char primeiro[64];
	memset(primeiro, 0x11, sizeof(primeiro));
	s.requestSendBuffer(primeiro, sizeof(primeiro));

	posted_op p;
	bool got = pool.pop(p);
	CHECK(got, "esperava um post inicial");
	if (got) {
		s.setSend();
		p.b->getWSABufToSend();
	}

	std::atomic<bool> terminou(false);
	std::thread t([&]() {
		unsigned char buf[1024];
		memset(buf, 0x22, sizeof(buf));
		for (int i = 0; i < 200; ++i)
			s.requestSendBuffer(buf, sizeof(buf));
		terminou = true;
	});

	for (int i = 0; i < 500 && !terminou; ++i)
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

	CHECK(terminou.load(), "requestSendBuffer BLOQUEOU com o send em voo (o bug nao foi corrigido)");
	t.join();

	s.releaseSend();
	io_sim io;
	while (pool.pop(p)) io.run_one(p);
	printf("      200 sends enfileirados sem bloquear; drenou %zu bytes depois\n", io.stream.size());
}

static void teste_concorrencia(mock_pool& pool, test_session& s) {
	printf("[4] 8 threads mandando + drenagem concorrente\n");

	const int NTH = 8, NMSG = 300, TAM = 200;
	std::atomic<int> vivos(NTH);
	std::atomic<bool> parar(false);
	io_sim io;
	std::mutex io_mtx;

	std::thread drena([&]() {
		posted_op p;
		while (!parar || pool.size() > 0) {
			if (pool.pop(p)) {
				std::lock_guard<std::mutex> lk(io_mtx);
				io.run_one(p);
			} else {
				std::this_thread::sleep_for(std::chrono::microseconds(200));
			}
		}
	});

	std::vector<std::thread> prods;
	for (int t = 0; t < NTH; ++t) {
		prods.emplace_back([&, t]() {
			std::vector<unsigned char> pkt(TAM);
			for (int m = 0; m < NMSG; ++m) {

				pkt[0] = (unsigned char)t;
				pkt[1] = (unsigned char)(m & 0xFF);
				pkt[2] = (unsigned char)((m >> 8) & 0xFF);
				for (int k = 3; k < TAM; ++k) pkt[k] = (unsigned char)(t * 31 + k);
				s.requestSendBuffer(pkt.data(), pkt.size());
			}
			--vivos;
		});
	}
	for (auto& th : prods) th.join();
	parar = true;
	drena.join();

	size_t total_esperado = (size_t)NTH * NMSG * TAM;
	CHECK(io.stream.size() == total_esperado, "perdeu ou duplicou bytes com varias threads");

	int msgs_ok = 0, msgs_ruins = 0;
	for (size_t off = 0; off + TAM <= io.stream.size(); off += TAM) {
		unsigned char t = io.stream[off];
		bool ok = (t < NTH);
		if (ok) {
			for (int k = 3; k < TAM; ++k) {
				if (io.stream[off + k] != (unsigned char)(t * 31 + k)) { ok = false; break; }
			}
		}
		if (ok) ++msgs_ok; else ++msgs_ruins;
	}
	CHECK(msgs_ruins == 0, "mensagem CORROMPIDA/entrelacada no stream");
	printf("      %d mensagens intactas, %d corrompidas, %zu bytes\n", msgs_ok, msgs_ruins, io.stream.size());
}

static void teste_overflow(mock_pool& pool, test_session& s) {
	printf("[5] estouro da fila derruba so o cliente lento\n");

	unsigned char primeiro[64];
	memset(primeiro, 0x33, sizeof(primeiro));
	s.requestSendBuffer(primeiro, sizeof(primeiro));
	posted_op p;
	if (pool.pop(p)) { s.setSend(); p.b->getWSABufToSend(); }

	std::vector<unsigned char> gordo(64 * 1024, 0x44);
	int mandados = 0;
	for (int i = 0; i < 64; ++i) {
		s.requestSendBuffer(gordo.data(), gordo.size());
		++mandados;
	}

	CHECK(mandados == 64, "requestSendBuffer travou ou lancou no estouro");

	size_t antes = pool.size();
	s.requestSendBuffer(gordo.data(), 100);
	CHECK(pool.size() == antes, "continuou postando send depois de estourar a fila");
	printf("      %d sends aceitos sem travar, session cortada como esperado\n", mandados);
}

static void teste_do_harness() {
	printf("[0] autoteste do harness (o CHECK acusa mesmo?)\n");

	int antes = g_fail;
	CHECK(1 == 2, "(esperado) esse aqui TEM que falhar -- e o autoteste");
	bool acusou = (g_fail == antes + 1);
	g_fail = antes;

	if (!acusou) {
		printf("  [FALHOU] o CHECK nao acusou uma condicao falsa: o harness esta quebrado\n");
		++g_fail;
	} else {
		printf("      ok, o CHECK acusa condicao falsa\n");
	}
}

int main() {
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	printf("=== harness da fila de saida (session::buff_ctx) ===\n");
	printf("    MAX_SEND_QUEUE = %lu bytes\n\n", (unsigned long)STDA_SESSION_MAX_SEND_QUEUE);

	teste_do_harness();

	struct caso { const char* nome; void (*fn)(mock_pool&, test_session&); };
	caso casos[] = {
		{ "ordem",        teste_ordem_simples },
		{ "raw",          teste_raw_nao_mistura },
		{ "nao bloqueia", teste_nao_bloqueia },
		{ "concorrencia", teste_concorrencia },
		{ "overflow",     teste_overflow },
	};

	for (auto& c : casos) {
		SOCKET sv = INVALID_SOCKET, cl = INVALID_SOCKET;
		if (!make_pair(sv, cl)) { printf("nao conseguiu criar o par de sockets\n"); return 2; }

		SOCKADDR_IN addr; memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

		mock_pool pool;
		test_session s(pool, sv, addr, 7);
		s.setState(true);
		s.setConnected(true);

		c.fn(pool, s);

		::closesocket(sv);
		::closesocket(cl);
	}

	printf("\n=== %s ===\n", g_fail == 0 ? "TUDO PASSOU" : "TEM FALHA");
	return g_fail == 0 ? 0 : 1;
}
