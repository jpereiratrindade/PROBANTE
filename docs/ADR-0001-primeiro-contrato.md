# ADR-0001 — Primeiro contrato externo

- Estado: aceito para v0.1.0
- Data: 2026-10-14

## Decisao

Usar contrato JSON versionado para CLI de soma `int64`, validado estritamente antes de qualquer lancamento. Separar `contract`, `executor`, `oracle` e `evidence`. Usar Boost.JSON 1.90, OpenSSL EVP e Bubblewrap fail-closed. Executar snapshot privado somente leitura, argumentos vetoriais via `execve`, ambiente `LC_ALL=C`, stdin `/dev/null`, rede desabilitada, 2 s/caso e 64 KiB/fluxo.

## Razao

Problema pequeno permite aferir o instrumento com bordas, entradas invalidas, bytes exatos, timeout, excesso de saida, processo defeituoso e isolamento, sem confundir complexidade do alvo com qualidade do verificador.

## Consequencias e limites

Linux e Bubblewrap tornam entrega nao portavel. Montagens de runtime do host sao somente leitura, mas aumentam superficie visivel. Namespace nao substitui auditoria de sandbox. JSON Schema documenta intercambio; validacao normativa tambem e aplicada pelo carregador, sem novo motor de schema. Nenhuma abstracao para familia futura foi criada.

