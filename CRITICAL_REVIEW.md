## Critical Technical and Product Review — Crawling Spider (C)

Date: 2025-08-09

### Executive summary
- **What it is**: A minimal C-based, libcurl+libtidy, multi-threaded same-domain crawler with file-based pause/resume.
- **Verdict for an AI-era product**: Not a viable base for a modern AI product. Keep as a learning artifact; **pivot** for any production intent.
- **Why**: Fundamental architectural, correctness, security, and DX gaps; difficult to evolve in C for modern crawling/AI ingestion needs. Rewriting in Go/Rust/Python will be faster, safer, and far more capable.
- **If you continue in C**: Substantial rework is required (concurrency, parsing, robots/politeness, URL normalization, resilience, testing, packaging). The effort exceeds the cost of starting clean with a more suitable stack.

---

### Project overview
- Language/stack: **C**, `libcurl`, `libtidy`, `pthreads`, file-based persistence.
- Core flow: interactive boot → first crawl → N worker threads → extract `<a href>` → filter to same-domain → in-memory hash for dedupe → queue → repeat → on signal, dump hash+queue to files.
- Code size: ~700 SLOC (excluding README).

---

### Technical audit (critical points with evidence)

#### 1) Global state defined in headers (linker ODR violation)
- `hashqueue.h` defines globals, causing multiple definitions across translation units:
  ```startLine:endLine:hashqueue.h
  #define MAX_HASH 10000
  //hash table
  node_t *hash_table[MAX_HASH];
  //declare a queue
  queue_t q;
  ```
  - Should be `extern` in headers and defined once in a `.c` file. As-is, correct linkers should fail or produce UB.

#### 2) Concurrency: incorrect locking and data races
- Two different mutexes guard the same shared resources (`q`, `hash_table`), which leads to races:
  ```startLine:endLine:spider.c
  pthread_mutex_lock(&mutex); url=dequeue(&q); pthread_mutex_unlock(&mutex);
  ...
  pthread_mutex_lock(&mutex2); crawl_frontier(head); pthread_mutex_unlock(&mutex2);
  ```
- `insert_hash()` mutates `hash_table` and enqueues into `q` without a consistent lock protocol. `enqueue()` itself has no internal locking.
- Net effect: potential corruption, lost updates, heisenbugs under load.

#### 3) Signal handling is unsafe and may corrupt state
- Signal handlers call non-async-signal-safe functions (I/O, heap functions, locks indirectly), which is undefined behavior:
  ```startLine:endLine:main.c
  signal(SIGINT, file_writer);
  signal(SIGSEGV, file_writer);
  signal(SIGABRT, file_writer);
  ...
  void file_writer(){
      file_writer_default();
      raise (SIGTERM);
  }
  ```
- Correct approach: use `sigaction` + set a flag and perform cleanup in a safe context, or use dedicated checkpointing.

#### 4) URL parsing, normalization, and extraction are brittle
- Magic numeric IDs for tags/attrs (fragile to libtidy changes); no normalization, canonicalization, or redirect handling:
  ```startLine:endLine:crawler.c
  if(tagId == 1) { /* a-tag ID = 1 */
    ... if(attrId == 58) /* href */
  }
  ```
- `crawl_frontier.c` builds URLs via string concatenation with assumptions (e.g., forced `https://`, naive `//` handling), leaking memory via `strdup()` without free, and missing normalization (trailing slashes, fragments, cases, queries, redirects).

#### 5) Domain extraction has an off-by-one bug and brittle logic
- Buffer overflow risk:
  ```startLine:endLine:bootup.c
  char tmp[7];
  strncpy(tmp, url, 6);
  tmp[7] = '\0'; // writes past end of tmp
  ```
- `http`/`https` detection is ad hoc; better to use a robust URI parser.

#### 6) Memory management: systemic leaks
- `dequeue()` never frees nodes; `spider.c` never calls `curl_easy_cleanup()` per thread; `crawl_frontier()` allocates via `strdup()` and never frees `temp`; list nodes inserted then freed only partially. Over a long crawl, the process will bloat.

#### 7) HTTP stack and crawler etiquette gaps
- No robots.txt or `sitemap.xml` support; no `User-Agent`, no rate limiting, no crawl-delay, no per-host politeness queues, no retries/backoff, no redirect following, no TLS settings, no compression toggles, no content size/time limits.

#### 8) Persistence model is fragile and lossy
- Full-dump on signal to flat files; no journaling or crash-safety; hash dump order nondeterministic; queue dump iterates by mutating a copy of the queue struct (works but fragile). No versioning or metadata for resume.

#### 9) CLI/UX unsuitable for automation
- Fully interactive (`scanf`, `system("clear")`). No CLI flags, no config files, no logs, no metrics, no structured output.

#### 10) Build, portability, and maintainability
- No Docker, no CI, no tests, no linting, no sanitizer builds. Linux-only assumptions. `mkdir(proj_name, 777)` uses decimal `777` not octal `0777` and immediately `chmod` to user-only, which is inconsistent and likely unintended.

---

### Security and compliance risks
- Disregards robots.txt ⇒ potential ToS violations and legal exposure.
- Lacks rate limiting ⇒ easy to DDoS targets.
- No domain allow/deny rules beyond naive same-domain filter.
- Interactive input via `scanf("%s", ...)` without bounds checking ⇒ buffer overflow risk.
- Signal-based persistence may corrupt files on crashes.

---

### Performance and scalability
- Single-process in-memory frontier with naive hashing; no sharding or external queue (e.g., Redis/Kafka).
- `libtidy` HTML parsing is CPU-bound; no adaptive backpressure.
- Concurrency limited to a small fixed thread count; no async I/O.
- No monitoring (QPS, fetch latency, error rates, memory growth).

---

### Documentation and DX
- README is clear for basic use; lacks production guidance, configuration, and operational playbooks.
- No examples, tests, or benchmarks.

---

### Product potential in the modern AI era
The raw capability (fetch links recursively) is a tiny subset of what modern AI data products require. However, the domain (web crawling/ingestion) is valuable if re-scoped:

- **AI-ready web ingestion pipeline**: Crawl → dedupe → extract → chunk → embed → store → serve for RAG/summarization. Requires robust crawling, parsing, JS rendering, storage, and LLM integration.
- **Focused semantic crawler**: Use an LLM to score links and prioritize crawling pages relevant to a topic/entity/intent. Feedback loop improves recall/precision.
- **Change monitoring + AI summaries**: Track deltas on target sites; generate summaries/newsletters/slack alerts.
- **SEO/Compliance auditor with AI insights**: Crawl a site, detect issues (broken links, accessibility, Core Web Vitals proxies), and auto-generate prioritized remediation plans.
- **Domain-specific data extractor**: Templates + LLM few-shot extraction to structured records (jobs, real estate, research papers), with dedupe, PII checks, and license awareness.
- **Training data curation for LLMs**: High-quality crawl with license filters, dedupe (SimHash/MINHASH), toxicity/PII filters, and dataset cards.

The current C codebase is not a strong foundation for any of these without major rewrites.

---

### Keep vs. abandon
- **Keep (as-is) if**: Goal is educational—systems programming, threading, low-level networking. Use it as a teaching lab and incrementally harden.
- **Abandon as a product base**: For a production-grade AI ingestion/crawling product, **pivot** to a modern stack and architecture. You will ship faster and safer.

---

### Recommended directions and stacks

#### Direction A: AI-ready crawler + RAG data plane (recommended)
- **Use**: Go (or Rust) for crawling/queueing; Python for AI services.
- **Crawling**: Go `colly` or `chromedp` (JS), with per-host rate limits, robots respect, retries, canonicalization.
- **Queue**: Redis/Kafka; **storage**: Postgres (+pgvector) or OpenSearch; **object store**: S3-compatible.
- **Parsing**: Readability algorithms; boilerplate removal; HTML → Markdown.
- **AI**: Embeddings + chunking; LLM summarization and extraction; guardrails; evaluator.
- **Serve**: FastAPI/Go HTTP, simple search API; observability (Prometheus/Grafana).

#### Direction B: Focused semantic crawler
- Add LLM link scoring (policy model); crawl budget; reinforcement via click-through or label feedback.

#### Direction C: Site auditor / change monitor
- Fast crawl of a single domain; diffing; AI-generated weekly reports.

---

### Minimum work to salvage this C code (if you insist)
- Fix global state (move definitions to a `.c`; `extern` in headers).
- Unify locking; guard `q` and `hash_table` with one mutex or per-structure locks; add thread-safe queues.
- Replace magic IDs with robust HTML parsing; canonicalize URLs; follow redirects; set `User-Agent`.
- Implement robots.txt and per-host politeness with rate limiting.
- Harden `extract_root`; fix off-by-one; use a URI parser.
- Free all allocations; RAII-like patterns; add `curl_easy_cleanup()` and tidy cleanups on all paths.
- Replace interactive CLI with flags/config; add logging.
- Containerize; add tests (unit + integration), sanitizers, CI.

Effort: multiple weeks for one engineer; still won’t match the capabilities of a Go/Rust/Python solution using mature libraries.

---

### Actionable immediate steps

1) Decide the path
- If product-oriented: sunset this C codebase as a base. Archive it as a learning project.

2) Spin up a modern scaffold (1–2 days)
- Create a new repo with two services:
  - `crawler` (Go): Colly-based crawler with robots.txt, per-host rate limits, redirect handling, canonicalization, dedupe (Bloom + Redis), and JS option via `chromedp` toggle.
  - `ingestion` (Python/FastAPI): Chunking, embeddings, summaries, and storage (Postgres + pgvector or OpenSearch). Simple search API.
- Add Dockerfiles, docker-compose, Makefile, pre-commit, CI.

3) Implement an MVP (3–5 days)
- Crawl a single domain depth-limited to N; store pages (raw + cleaned) and normalized URLs. Respect robots and politeness.
- Generate embeddings and store in vector DB. Provide `/search` and `/summarize?url=` endpoints.
- Add a minimal web UI to test search and summaries.

4) Hardening and telemetry (ongoing)
- Add retries/backoff, content-type/size/time limits, MIME sniffing, dedupe hashes, canonicalization rules.
- Add metrics (fetch latency, success rate, q depth), logs, and dashboards.
- Integrate proxy rotation if needed.

5) If you still want to incrementally improve this repo (short-term hygiene)
- Fix the header globals by moving definitions into a new `state.c`:
  - `queue_t q; node_t* hash_table[MAX_HASH];` defined in `state.c`; `extern` declared in headers.
- Replace unsafe I/O: use `fgets` and bounded `sscanf` or `strncpy`.
- Correct `extract_root` buffer bug and use proper octal in `mkdir(…, 0777)`.
- Remove signal handler file I/O; instead, checkpoint periodically on a timer in the main loop.
- Add `User-Agent`, `CURLOPT_FOLLOWLOCATION`, timeouts, and `curl_easy_cleanup()` in `spider.c`.

---

### Appendix: Notable code references
- Multiple definitions in header:
  ```startLine:endLine:hashqueue.h
  queue_t q;
  node_t *hash_table[MAX_HASH];
  ```
- Off-by-one and brittle domain parsing:
  ```startLine:endLine:bootup.c
  char tmp[7]; strncpy(tmp, url, 6); tmp[7] = '\0';
  ```
- Magic IDs for tag/attr selection:
  ```startLine:endLine:crawler.c
  if(tagId == 1) { /* a */ ... if(attrId == 58) { /* href */ }
  ```
- Unsafe signal handler performing file I/O:
  ```startLine:endLine:main.c
  signal(SIGINT, file_writer); ... void file_writer(){ file_writer_default(); raise(SIGTERM); }
  ```