# 🤖 RAG 混合检索知识库体系构建实践

## 文档说明

## 环境说明与版本

| 系统角色 | 操作系统 | CPU 数量 | 内存容量 | 数据磁盘容量 | Docker | Docker Compose | Milvus | Prometheus | Alertmanager |
| ----- | ----- | ----- | ----- | ----- | ----- | ----- | ----- | ----- | ----- |
| **RAG 知识库系统** | Rocky Linux 9.8 | 4 | 16G | 300G | 29.5.0 | v5.1.3 | v2.6.16 | - | - |
| **Milvus 数据库** | Rocky Linux 9.8 | 8 | 32G | 300G | 29.5.0 | v5.1.3 | v2.6.16 | - | - |

💥 注意：Milvus 向量数据库至少需要 8G 可用内存才能运行，低于此内存 milvus-standlone 容器启动失败！

## 文档目录

- [🤖 RAG 混合检索知识库体系构建实践](#-rag-混合检索知识库体系构建实践)
  - [文档说明](#文档说明)
  - [环境说明与版本](#环境说明与版本)
  - [文档目录](#文档目录)
  - [🧬 RAG 原理图：RAG 在线查询的混合检索分离架构](#-rag-原理图rag-在线查询的混合检索分离架构)
  - [🪄 RAG 项目架构图](#-rag-项目架构图)
  - [📡 RAG 项目各组件调用时序图](#-rag-项目各组件调用时序图)
  - [🖥️ RAG 知识库系统主机准备](#️-rag-知识库系统主机准备)
  - [🐳 Docker 容器引擎部署与管理](#-docker-容器引擎部署与管理)
  - [🐍 Python 开发环境与依赖包设置](#-python-开发环境与依赖包设置)
  - [🦆 Docling 组件准备](#-docling-组件准备)
    - [~/.cache/huggingface/hub/ 中的 Docling 模型](#cachehuggingfacehub-中的-docling-模型)
    - [rapidocr\_onnxruntime 自带的 OCR 模型](#rapidocr_onnxruntime-自带的-ocr-模型)
    - [💡 Docling 各模型的功能分工](#-docling-各模型的功能分工)
  - [🎉 RAG 知识库项目结构解析](#-rag-知识库项目结构解析)
  - [🍭 RAG 分块策略汇总](#-rag-分块策略汇总)
  - [🗄️ Milvus 向量数据库多容器部署实践](#️-milvus-向量数据库多容器部署实践)
  - [🎨 使用 zilliz/attu 客户端连接 Milvus 向量数据库](#-使用-zillizattu-客户端连接-milvus-向量数据库)
  - [📤 多类型格式文件索引](#-多类型格式文件索引)
  - [🥏 RAG 运行检索测试](#-rag-运行检索测试)
  - [📈 RAG 效果评估](#-rag-效果评估)
    - [检索环节评估](#检索环节评估)
    - [生成环节评估](#生成环节评估)
    - [RAG 幻觉问题与应对](#rag-幻觉问题与应对)
    - [RAG 优化与评估的一体化思路](#rag-优化与评估的一体化思路)
  - [📦 RAG 应用容器镜像封装](#-rag-应用容器镜像封装)
  - [🩺 RAG 系统监控部署](#-rag-系统监控部署)
  - [RAG 系统功能调试](#rag-系统功能调试)
    - [OCR 格式的 PDF 文件判断](#ocr-格式的-pdf-文件判断)
    - [调试 rerank 与  prompt 构建返回](#调试-rerank-与--prompt-构建返回)
    - [获取 SQLite3 数据库的查询与性能数据](#获取-sqlite3-数据库的查询与性能数据)
  - [📜 附录：RAG 项目优化过程](#-附录rag-项目优化过程)
  - [📚 参考链接](#-参考链接)

## 🧬 RAG 原理图：RAG 在线查询的混合检索分离架构

文档入库 → 分块 → 向量化 + metadata → 用户 Query →（可选）Query 改写 → 稠密 + 稀疏混合检索 → Rerank → 截断 → Prompt 组装 → 生成 → 引文溯源 / 合规校验

## 🪄 RAG 项目架构图

## 📡 RAG 项目各组件调用时序图

## 🖥️ RAG 知识库系统主机准备

## 🐳 Docker 容器引擎部署与管理

## 🐍 Python 开发环境与依赖包设置

## 🦆 Docling 组件准备

为了让 Docling 能有效识别 PDF 版面组织、表格化结构以及 OCR 扫描格式，需要准备以下相关模型与模块：

### ~/.cache/huggingface/hub/ 中的 Docling 模型

- models--docling-project--docling-layout-heron — 版面检测模型
- models--docling-project--docling-models — TableFormer 等结构识别模型

这些模型不是随 pip install docling 一起装进来的，而是首次调用 DocumentConverter 时通过 huggingface_hub 从 HF 上按需下载，或者用 docling 自带的 CLI 一次性预拉取到 HF 的标准缓存目录（$HF_HOME/hub，默认就是 ~/.cache/huggingface/hub/）。两种做法：

方式 A — 预下载（推荐，离线/生产环境）：

```bash
$ source .rag-runtime/bin/activate
$ pip install "docling>=1.0.0"    # 已在 requirements.txt
# 或： pip install docling-tools
$ docling-tools models download   # 拉全部默认模型
# 只拉需要的几个：
$ docling-tools models download layout tableformer    # 推荐用法
```

方式 B — 首次运行时自动下载：

只要 pip install docling 装好，并且机器能访问 huggingface.co（或设置了镜像 HF_ENDPOINT=https://hf-mirror.com），第一次跑 DocumentConverter().convert(pdf) 时 docling 会自己把上面那两个 repo snapshot_download 下来，结果就在目录结构 models--<org>--<repo>/snapshots/<commit>/… 中。

指定缓存位置可以设 HF_HOME 或 HF_HUB_CACHE；离线运行时设 HF_HUB_OFFLINE=1。

### rapidocr_onnxruntime 自带的 OCR 模型

RapidOCR 的检测/方向分类/识别三个 ONNX 模型是直接打包在 wheel 里的，不需要额外从网上下载：

```bash
$ pip install "onnxruntime>=1.16.0"           # ONNX 运行时后端
$ pip install "rapidocr_onnxruntime>=1.4.0"   # OCR 本体，模型跟随一起装
```

装完模型文件位于 site-packages/rapidocr_onnxruntime/models/（det/cls/rec 三个 .onnx），Docling 探测到 rapidocr_onnxruntime 可 import 就会启用它，全程离线可用。GPU 加速可换成 onnxruntime-gpu。

### 💡 Docling 各模型的功能分工

Docling 的 PDF 处理是三层流水线，各干各的：

| 层 | 模型 | 位置 | 作用 |
| ----- | ----- | ----- | ----- |
| 1️⃣ 版面分析 | docling-layout-heron (164M) | ~/.cache/huggingface/hub/models--docling-project--docling-layout-heron | 在每页图像上画框：识别哪块是 title / section_header / paragraph / table / picture / page_header。这是 "结构感知分块" 和 section_path 之所以能生效的根本原因。 |
| 2️⃣ 表格结构 | docling-models/tableformer (342M) | .../docling-models/snapshots/.../model_artifacts/tableformer/fast/, accurate/ | 拿到 "这块是表格" 的框后，进一步识别表格的行列结构（哪几个单元格属于同一行、跨列合并等），才有 export_to_markdown() 里正确的 markdown 表格。 |
| 3️⃣ 文字识别 | rapidocr PP-OCRv6 (~10M，随包带) | .rag-runtime/lib/.../rapidocr/models/ | 仅在文本抽取失败时（图片型 PDF、扫描件）才被调用，从像素里识别文字。 |

也就是说：

- 文本型 PDF：层 1️⃣+2️⃣ 用到，层 3️⃣ 不用。
- 扫描/图片 PDF：层 1️⃣+2️⃣+3️⃣ 都用。

RapidOCR 只负责第三步 "字变文本"，替代不了前两步。

测试扫描件时输出的这行：Loading weights: 100%|██████████| 770/770 [00:00<00:00, 2697.47it/s]

就是 tableformer 加载权重（770 层）—— 即使那份测试 PDF 没有表格，只要 do_table_structure=True，模型就会被加载做 "这一页里有没有表格" 的检测。RapidOCR 那三个 .onnx 加载在它上面几行，是另外一码事。

能不能省这 500M？

理论上可以，代价明显：

- 删 docling-layout-heron（164M）：Docling 的 StandardPdfPipeline 强依赖 layout 模型；删了会直接报错、fallback 到 PyMuPDF，你就丧失了 "结构感知分块 + section_path + 表格识别"。
- 删 docling-models/tableformer（342M）：如果你把 pipeline_options.do_table_structure = False，可以不加载它。但那样 Docling 不会输出表格结构，你的 "表格双表示" 也就无从谈起。表格 chunk 会变成一段没格式的文字流。
- 删 rapidocr 模型：随包发布，删了下次 pip install rapidocr_onnxruntime 又回来。

建议保留现状。三层加起来约 500M，对比它们提供的能力（结构感知 + 表格结构 + OCR）非常划算。

## 🎉 RAG 知识库项目结构解析

## 🍭 RAG 分块策略汇总

| 分块策略 | 策略描述 | 优点 | 缺点 | 适用场景 |
| ----- | ----- | ----- | ----- | ----- |
| 固定分块 | 按照固定字符数分块，例如每 500 个 token 分一块，相邻块的边界处重叠 100 个 token（冗余设计，缓解语义截断问题）。 | 实现简单，速度快。 | 容易拆断句子，破坏语义完整性。 | 简单的 FAQ 文档，对检索精度要求不高。 |
| 递归分块 | 先按段落分块，如果段落太大，再按句子分块，直到分块大小符合要求。 | 尽量保留段落与层级的语义完整性。 | 对于结构复杂的文档，仍然会有语义截断。 | 有层级结构的文档，例如法律手册、金融报告和技术博客等。是大多数 RAG 项目的首选。 |
| 语义分块 | 通过余弦相似性衡量相邻句子的语义关联性。若相似度高，则合为一块；一旦相似度低于下限，即视为语义边界，分为两块。 | 保证每个分块的语义是一致的，极大提升检索准确性。 | 需要计算语义关联性，成本高。 | 长文和深度内容。 |
| 结构分块 | 按照特定的文档结构分块。例如 Markdown 按照标题层级分块；HTML 按照标签层级分块；PDF 按照页或章节分块；代码按照函数、类或包分块。 | 当文档有清晰的结构时，这种结构本身就是天然的语义边界，这种情况下结构分块是最靠谱的。 | 依赖文档有清晰的结构。 | 有特定结构的文档，例如金融或法律等有清晰章节的文档、代码、Makedown、HTML 等。 |
| 父子分块 | 无论是哪种分块策略，都会遇到一个矛盾：小块检索精准但上下文缺失，大块上下文完整但检索不准。父子分块的解题思路是先把文档分成小块，再把小块关联到它的父级段落（大块）。检索时先命中小块，再把对应的父级段落一起放入 Prompt。 | 结合了小块检索精准和大块上下文完整的优势。 | 增加索引内存和多一次关联查询。 | 长文和深度内容。 |

## 🗄️ Milvus 向量数据库多容器部署实践

## 🎨 使用 zilliz/attu 客户端连接 Milvus 向量数据库

## 📤 多类型格式文件索引

## 🥏 RAG 运行检索测试

## 📈 RAG 效果评估

### 检索环节评估

| 指标 | 含义 |
| ----- | ----- |
| 召回率 Recall@K | 检索到的相关文档数 / 实际相关文档数 |
| 平均倒排率 MRR | 相关文档的平均排名倒数 |
| NDCG（归一化折损累积增益）| 综合考虑文档相关性与排序位置 |
| Precision@K | 前 K 条结果中相关文档比例 |
| Coverage / Diversity | 检索结果的主题多样性 |

✅ 优秀的 RAG 模型在 Recall@K 和 MRR 上表现较高，同时保持检索结果的多样性。

### 生成环节评估

| 指标 | 含义 |
| ----- | ----- |
| Rouge-L / BLEU | 文本与参考答案的 n-gram 重叠度 |
| BERTScore / Embedding Similarity | 基于语义向量的文本相似度 |
| Factuality | 检查生成内容是否符合检索事实 |
| Diversity | 语言自然度与生成多样性 |
| Hallucination Rate | 模型幻觉（编造信息）的比例 |

### RAG 幻觉问题与应对

🌀 幻觉的来源：

- 检索阶段召回的文档不相关或缺失关键信息
- LLM 忽略检索内容、凭语言模式生成
- 检索文档存在歧义或事实错误
- Prompt 设计不当，边界约束不足。

💡 主要解决方案：

- 增强 Prompt 约束：明确指令 “请仅依据提供的文档回答”
- 引用标注机制：让模型输出时附带来源引用
- Fact-checking 模型：生成后自动事实核查
- Confidence Scoring：模型为自身输出打分，低置信度则再生成。

### RAG 优化与评估的一体化思路

优化目标：

- 检索更准：召回覆盖率高、排序准确
- 上下文更稳：文档干净、冗余少
- 生成更真：幻觉率低、语义连贯

评估目标：

- 通过多维指标量化 “召回—生成—事实一致性” 的全链路质量。
- 结合 自动评估 + 人类评估，实现持续闭环优化。

## 📦 RAG 应用容器镜像封装

## 🩺 RAG 系统监控部署

## RAG 系统功能调试

### OCR 格式的 PDF 文件判断

判断 PDF 是不是 "扫描件/需要 OCR"，本质上是判断它每页有没有可提取的文本层。当前项目里已经有现成的依赖（docling / PyMuPDF），下面给两种方式，从最轻量到最贴近工程实践。

推荐，最快：用 PyMuPDF 直接看文本层

- unified_loader.py 已经引入 PyMuPDF 做回退，直接借用它的 page.get_text()：如果多页文本长度都接近 0，但页面有图，就是扫描/OCR PDF。
- 用法：

  ```bash
  # 单文件
  python scripts/detect_pdf_ocr.py 项目方案/202604-1386829.pdf

  # 整个目录（递归）
  python scripts/detect_pdf_ocr.py 产品文档 项目方案 常见疑难

  输出示例（kind 一列就是结论）：

  kind               pages  empty%  img%    chars  file
  text                  42    0.0%   4.8%   85342  产品文档/xxx.pdf
  scanned               18   94.4%  100.0%     37  项目方案/xxx.pdf
  mixed                 60   35.0%   60.0%   12048  产品文档/yyy.pdf
  ```

  1️⃣ text → 不需要 OCR，Docling 可以关掉 ocr_enabled 加速；</br>
  2️⃣ scanned / scanned_no_image → 必须开 OCR 才能提取内容；</br>
  3️⃣ mixed → 建议开 OCR（Docling 会自动只对无文本层的页跑 OCR）

- 判定阈值调整建议：
  - TEXT_MIN_CHARS=20：中文扫描件有时会残留几个乱码字符，20 比较稳；PDF 里全是英文数字表格的话可以放到 5。
  - IMAGE_RATIO=0.6：越大越严格（越难被判为扫描件）。对"每页头都有 logo 图但正文是文字"的模板类 PDF，用 0.6~0.7 比较合适。

### 调试 rerank 与  prompt 构建返回

### 获取 SQLite3 数据库的查询与性能数据

## 📜 附录：RAG 项目优化过程

- **优化1：在线检索生成时间严重超时**
  - 瓶颈1：RRF 融合结束后进行 rerank，如果融合后的条目越多，那么 rerank 花费较长时间（次要原因）。
  - 瓶颈2：RAGAS 评估整合在 web/app.py 中导致评估过程阻塞检索生成流程，最终造成整体查询时间远大于 120000ms (4分钟多)，用户体验很差。将 RAGAS 评估放在后台异步执行，Web 持续显示 SQLite3 数据库中的统计数据（主要原因）。
- **优化2：Web 侧边栏与对话框显示**
  - Logo 显示：添加顶部动态 Logo
  - 检索范围：手动添加检索范围选择框，提高检索生成准确率，依然保留自动选择功能。
  - 独立索引：手动指定多个 collection 索引文件数量，避免大量文件同时索引而造成 Milvus 高负载与 token 大量消耗。
  - 查询统计：添加从用户查询到结果返回消耗的总时长
  - RAGAS 评估：添加 RAG 系统评估指标
- **优化3：索引状态显示调整**
  - 多个 collections 索引中断问题：解决多个 collection 同时索引时，一个索引中断另一个索引的问题。
  - 始终显示索引状态：当刷新页面，索引进度立即消失，也无法显示索引完成度。采用多线程模型，将索引放置后台执行，索引状态也不放在 Web 内存中，而是直接从 SQLite3 数据库中再次读取显示在页面上。
- **优化4：Docling 有效处理 PDF 文件**
  - 表格处理优化：表头+首行，表格内容 Markdown，一并拼成 text，供 BM25 检索使用。
  - OCR 功能启用：OCR 格式文件处理，失败回退 PyMuPDF。
    - HuggingFace 模型下载
    - rapidocr 模块依赖模型下载
    - 项目代码要有实现

  > 注意：原始实现中未启用稀疏索引和表格的索引，丢失表格有效信息，也未启用 OCR 扫描件识别！

- **优化5：DOCX/PPTX/HTML 处理**
  - 多文件格式处理：.doc 格式文件多半不兼容 Docling，因此保留 python3-doc 模块处理。
- **优化6：FastAPI 多端点支持（暂未测试）**
  - 跨平台支持：兼容其他组件的 API 调用，需要 API Key 设置。
- **优化7：CSS 改写美化 Web 页面**
  - _theme.py 调整：新增 theme 程序调整前端 Streamlit 页面
- **优化8：最终返回回答优化**
  - 提高 top_n_rerank 数量
  - 提高塞进 prompt 的最大 token 数量
  - 提高生成模型的 token 数量

  > 注意：表格数据或跨多文件检索的结果可能需要更多的 chunk 才能准确的返回！

- 待解决问题：
  - 索引阶段：
    - Docling 处理 PDF 文件的逻辑是什么？
    - 如何使用当前项目中的类或方法对 PDF 文件做分块，返回分块后的结果？
    - 如何从 Milvus 中去除重复索引的文件数据？
    - 如何确认索引的文件对应哪些向量？
  - SQLite3 数据库操作： 
    - 如何对 SQLite3 进行 CRUD（sqlite3 命令行与 Python 程序）？

## 📚 参考链接

- [从0搭一套RAG系统，我踩的6个坑](https://mp.weixin.qq.com/s/AEtzeI-QrgHFjKWD1SZWGQ)
