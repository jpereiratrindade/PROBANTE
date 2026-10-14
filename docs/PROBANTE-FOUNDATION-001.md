# PROBANTE-FOUNDATION-001

## Constituicao

PROBANTE mede propriedades observaveis usando contrato externo, estimulo controlado, oraculo independente e evidencia reproduzivel. Principio: **Sempre pronto. Sempre incompleto.**

## Invariantes

1. Contrato precede exame do alvo e nao deriva expectativas dele.
2. Executor nao decide conformidade; oraculo nao executa; evidencia nao cria fatos.
3. Alvo e tratado como caixa-preta e executado a partir de snapshot privado.
4. Falha de isolamento fecha em `BLOCKED`; nunca ha fallback no host.
5. Bytes, exit code, limites e contexto efetivamente observados fundamentam decisoes.
6. `PASS` nao equivale a certificacao, ausencia de defeitos ou aprovacao do sistema.

## Escopo atual

Uma familia: FAM-001, teste funcional de CLI. Um contrato: soma decimal assinada de 64 bits. Doze casos canonicos. Linux apenas.

## Nao objetivos

GUI, servidor, API, MCP, LLM, banco, catalogo, certificado, descoberta de projetos, integracoes, multiplataforma, testes internos do alvo, desempenho ou seguranca gerais. Licenca pendente. Publicacao remota proibida sem decisao posterior.

