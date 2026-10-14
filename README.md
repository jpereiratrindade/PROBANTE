# PROBANTE

Infraestrutura de Verificacao Independente e Evidencias de Software.

> **Sempre pronto. Sempre incompleto.**

Esta entrega qualifica somente FAM-001: verificacao funcional, externa e de caixa-preta de uma CLI de soma `int64`. `PASS` limita-se ao contrato, casos, artefato e ambiente registrados; nao aprova, certifica nem atesta qualidade global do software.

## Dependencias e ambiente suportado

- Linux; CMake >= 3.30; compilador com C++26 real;
- Boost.JSON **1.90** (`Boost::json`). Se ausente, CMake baixa release oficial e valida SHA-256;
- OpenSSL >= 3 (`OpenSSL::Crypto`);
- Bubblewrap (`bwrap`) rootless com namespaces user/PID/mount/network. Ausencia ou falha produz `BLOCKED`, nunca execucao no host.

Versoes usadas no gate ficam em `docs/GATE-FAM-001.md`. Primeira configuracao pode exigir rede apenas para obter Boost.JSON; execucoes dos alvos ficam sem rede.

## Reproducao

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
./build/probante verify \
  --target ./build/fixtures/int64-add-correct \
  --contract ./contracts/int64-add-cli.v1.json \
  --evidence-dir .runs
```

Saida da CLI e JSON. Codigos: `0 PASS`, `1 FAIL`, `2 BLOCKED`, `3 INVALID_REQUEST`. Evidencia de cada solicitacao fica em subdiretorio privado, contendo `result.json`, `manifest.json` e fluxos brutos por caso. Para conferir hashes:

```bash
sha256sum .runs/<request-id>/snapshot/target .runs/<request-id>/cases/*
```

Fixtures adversariais ficam em `build/fixtures`. Todos usam o mesmo endpoint publico; exemplos completos, expectativas, adulteracao e repeticao estao no gate e na suite CTest.

## Limites

Somente fixtures sinteticas deste repositorio foram autorizadas. Bubblewrap reduz acesso, mas nao prova resistencia universal contra codigo malicioso ou escape. Hash SHA-256 demonstra integridade relativa ao valor registrado, nao origem, autoria ou assinatura. Licenca permanece pendente; repositorio e local e nao publicado. Nao ha FAM-002.

