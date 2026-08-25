# RAMFlux Roadmap — Rumo ao Melhor Orquestrador de Memória do Windows

> Visão: ser o `Process Lasso + RAMMap + ISLC` em um só, mas inteligente, seguro e comprovadamente eficaz.

## Matriz Esforço x Impacto

| Ideia | Impacto | Esforço | Fase |
|-------|---------|---------|------|
| CI: windeployqt + teste MSI em VM limpa | Alto (evita bug Brotli) | Baixo | 2.52 |
| Portable ZIP + CLI `--json` | Alto | Baixo | 2.52 |
| Off-UI thread (M1/M2) | Alto (fim dos travamentos) | Médio | 2.52 |
| Standby por prioridade (não só limpar) | Muito Alto | Médio | 2.52 |
| Reversão transacional + teste de caos | Alto | Médio | 2.52 |
| Timeline de memória + explicabilidade | Alto | Médio | 2.53 |
| HeuristicEngine preditivo (60s) | Muito Alto | Alto | 2.53 |
| Perfis auto-aprendidos por exe | Muito Alto | Alto | 2.53 |
| Métrica "MB sem hard faults" | Alto | Baixo | 2.53 |
| Dedup COW real | Muito Alto | Muito Alto | 3.0 |
| NUMA/cache-aware dinâmico | Alto | Alto | 3.0 |
| WSL2/vmmem + VRAM orchestration | Médio | Médio | 3.0 |

---

## Fase 1 — v2.52 Fundação Sólida (2-3 semanas)

### 1. Confiabilidade Primeiro
- [ ] **CI blindado:** `windeployqt6` no `build2` + `gh actions` que instala MSI em `windows-latest` limpo e roda `RAMFlux.exe --benchmark`
- [ ] **Reversão 100%:** teste de caos — matar processo no meio de `applyGameOptimizations` e garantir restore. Fuzz em `MemoryQoS`
- [ ] **Off-UI thread:** mover `NtApi::getPhysicalMemoryBreakdown/getCompressionStoreInfo` para `QThreadPool`, UI só consome `MemorySnapshot` via `EventBus`

### 2. Orquestração Inteligente (não burra)
- [ ] **Standby inteligente:** trocar `clearStandbyList()` cego por `NtSetSystemInformation(SystemMemoryListInformation)` com prioridades 0-7. Manter cache quente.
- [ ] **OfferVirtualMemory:** para processos com `coldPageBytes > 30%`, usar `OfferVirtualMemory` ao invés de `EmptyWorkingSet` — sem page fault na volta.

### 3. Distribuição
- [ ] **Portable ZIP:** `build2/deploy` zipado + `RAMFluxHelper` opcional
- [ ] **CLI headless:** `RAMFlux.exe --optimize --json --once` para servidores/scripts

---

## Fase 2 — v2.53 Inteligência (4-6 semanas)

- [ ] **Timeline:** gráfico empilhado `standby/modified/compressed/free` + marcadores "Jogo X detectado → cache 512MB"
- [ ] **Explicabilidade:** `Pressão 78% → Chrome 2.1GB standby + 120 faults/s`
- [ ] **Métrica honesta:** `BenchmarkRunner` reporta `hardFaultsDelta` e `standbyReclaimed`. Provar que não é placebo.
- [ ] **HeuristicEngine preditivo:** regressão linear em `faultTrend + standbyGB` → prever pressão 60s, agir antes
- [ ] **Perfis auto-aprendidos:** se `exe` tem `avgWorkingSet > 1GB` e `peak > 2GB` em 3 execuções, auto-criar regra `PAGE_PRIORITY_LOW` para background

---

## Fase 3 — v3.0 Domínio (2-3 meses)

- [ ] **Dedup COW:** `VirtualAlloc2` + `MEM_WRITE_WATCH` + hash FNV — único no Windows consumer
- [ ] **NUMA dinâmico:** migrar threads com `SetThreadGroupAffinity` baseado em `LLC miss` via `NtQuerySystemInformation`
- [ ] **VRAM + WSL2:** `getVideoMemoryInfo()` já existe — orquestrar `vmmem` e VRAM juntos. Limitar `WSL2` via `.wslconfig` dinâmico
- [ ] **Marketplace de regras:** export/import de `ProcessRules` + comunidade

---

## Como medir sucesso

- **Hard faults/s < 20** sob carga (vs >100 sem RAMFlux)
- **Standby reclaim > 500MB sem aumentar faults**
- **Game 1% low FPS +5-10%** (vs sem otimização)
- **0 crashes em 30 dias** em 100 máquinas (telemetria opt-in)

Quer priorizar uma fase? Abra uma issue com `Fase 1` label.
