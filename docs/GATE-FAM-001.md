# Gate FAM-001 — registro de qualificacao

Estado: **QUALIFIED para FAM-001**, em 2026-10-14. Conclusao limitada aos cenarios controlados, artefatos e ambiente abaixo. Nao aprova plataforma para uso irrestrito.

## Ambiente observado

- Fedora Linux, kernel `7.2.9-201.fc44.x86_64`;
- CMake 4.3.0; GCC 16.2.1, `CMAKE_CXX_STANDARD=26`, requerido, sem extensoes;
- Boost.JSON 1.90.0 obtido da distribuicao oficial com SHA-256 `49551aff3b22cbc5c5a9ed3dbc92f0e23ea50a0f7325b0d198b705e8ee3fc305`;
- OpenSSL 3.5.9; Bubblewrap 0.12.0;
- Bubblewrap estabeleceu namespaces user, PID, mount e network no ensaio pratico.

## Comandos executados e resultados

```text
git init -b main
# repositorio Git vazio criado; nenhum remoto configurado

cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel 2
# build completo, C++26 estrito, sem warnings (warnings sao erros)

ctest --test-dir build --output-on-failure
# 1/1 fam001-qualification passou; 100%; 25,80 s na rodada final

./build/tests/qualification/qualification.sh
# correct, M01-M08, output-limit, crash, spaces, repeat e tamper-detection: PASS

cmake -S . -B build-analyzer -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS=-fanalyzer
cmake --build build-analyzer --parallel 2
# analise estatica GCC concluida sem diagnosticos na rodada final

./build/probante verify --target ./build/fixtures/int64-add-correct --contract ./contracts/int64-add-cli.v1.json --evidence-dir .runs
# exit 0; PASS; 12/12; snapshot e isolamento registrados
```

## Defeitos encontrados durante qualificacao

1. URL inicialmente presumida para release isolada Boost.JSON retornou 404. Corrigida para arquivo oficial Boost 1.90 completo, com hash calculado e fixado.
2. Pacote isolado nao fornecia alvos das dependencias Boost. Build alterado para compilar `boost/json/src.hpp` contra distribuicao completa.
3. Compilador estrito encontrou indentacao ambigua e parametros nao usados. Codigo corrigido; flags nao reduzidas.
4. Primeira versao do script tinha declaracao Bash com expansao prematura. Corrigida e repetida.
5. GCC `-fanalyzer` sinalizou risco de descritor na fixture M06. Fixture passou a usar RAII; analise repetida sem diagnosticos.

## Matriz prevista

| Ensaio | Expectativa |
|---|---|
| correta | PASS, 12/12 |
| M01 | FAIL / STDOUT_MISMATCH |
| M02 | FAIL nas bordas / EXIT_CODE_MISMATCH ou STDERR_MISMATCH |
| M03 | FAIL nos invalidos |
| M04 | FAIL / STDOUT_MISMATCH |
| M05 | FAIL / TIMEOUT; grupo encerrado |
| M06 | sentinela hospedeira intacta; qualificacao de isolamento, nao correcao funcional |
| M07 | BLOCKED sem Bubblewrap/namespaces; nenhum caso lancado |
| M08 | INVALID_REQUEST antes do snapshot/lancamento |
| adicionais | caminho com espacos, crash, 64 KiB, adulteracao e repeticao detectados |

Resultado observado: todos conforme expectativa. Fixture correta: 12/12. M01 `STDOUT_MISMATCH`; M02 e M03 divergencias de exit code; M04 `STDOUT_MISMATCH`; M05 `TIMEOUT` e nenhum descendente remanescente; M06 sentinelas RO/host intactas e rede externa indisponivel; M07 `BLOCKED` com `cases=[]`; M08 `INVALID_REQUEST` e alvo-probe nao lancado. Excesso: `OUTPUT_LIMIT_EXCEEDED`. Crash: divergencia de exit code. Repeticao preservou classificacao. Alteracao de `result.json` mudou hash frente ao manifesto.

## Evidencia

Cada execucao cria diretorio modo `0700`; arquivos modo `0600` (snapshot executavel `0500`, sentinela `0400`). Contem contrato copiado, snapshot do alvo, streams brutos, `result.json` e `manifest.json`. Manifesto liga caminhos relativos a SHA-256; hashes sao rechecados apos escrita. Rodadas automatizadas: `build/qualification-runs/evidence/` (artefato local ignorado pelo Git). Rodada publica: `.runs/` (tambem ignorada).

## Criterios objetivos

1. build C++26 e versoes: satisfeito;
2. contrato estrito e invalidos: satisfeito;
3. fixture correta 12/12: satisfeito;
4. M01-M05: satisfeito;
5. M06-M08: satisfeito;
6. alvo independente, sem biblioteca/teste interno: satisfeito;
7. snapshot/hash ligados: satisfeito e revalidado;
8. repeticao deterministica: satisfeita;
9. evidencia legivel, hashes, casos e limites: satisfeita;
10. sem shell, rede, escrita em fonte ou fallback host: satisfeito por revisao e ensaio;
11. CTest e revisao: satisfeitos;
12. reproducao no README: satisfeita.

## Separacao semantica

- status de execucao: `STARTED` ou `NOT_STARTED` por caso;
- conformidade contratual: `PASS`, `FAIL`, `BLOCKED` ou `INVALID_REQUEST`;
- qualificacao do verificador: estado deste gate;
- qualidade global: **nao avaliada**.

## Revisao manual independente da execucao automatizada

Inspecao estatica confirmou: nenhum `system()`/`popen()`/shell; `execvp` recebe vetor; contrato e validado antes do snapshot/lancamento; ausencia de fallback host; executor nao contem expectativas nem nomes de fixtures; oraculo nao executa processos; fixtures nao incluem codigo PROBANTE; evidencia nao decide conformidade; expectativas canonicas conferem com FAM-001. GCC `-fanalyzer` forneceu verificacao separada e terminou sem diagnosticos. Limite: revisao feita nesta etapa, nao constitui auditoria especializada externa da fronteira Bubblewrap.

## Limitacoes permanentes

Somente Linux e fixtures sinteticas. Bubblewrap e kernel pertencem a base confiavel e nao foram auditados aqui. Montagens `/usr` e `/lib64` expõem runtime somente leitura. SHA-256 nao autentica origem. Casos controlados nao demonstram ausencia universal de falsos positivos ou negativos, nem conformidade integral com ISO/IEC/IEEE 29119.
