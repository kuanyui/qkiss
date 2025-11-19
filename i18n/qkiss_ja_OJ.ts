<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="ja_JP" sourcelanguage="en_US">
<context>
    <name>AboutDialog</name>
    <message>
        <location filename="../src/aboutdialog.cpp" line="47"/>
        <source>About QKiSS</source>
        <translation>QKiSSについて</translation>
    </message>
    <message>
        <location filename="../src/aboutdialog.cpp" line="75"/>
        <source>QKiSS is a modern implementation of [KiSS](https://ja.wikipedia.org/wiki/KISekae_Set_system) viewer in Qt 6.</source>
        <translation>QKiSSは、Qt 6にて開発されましたモダンな [KiSS](https://ja.wikipedia.org/wiki/KISekae_Set_system) ビューアでございますわ。</translation>
    </message>
    <message>
        <location filename="../src/aboutdialog.cpp" line="94"/>
        <source>
QKiSS is a modern implementation of [KiSS (Kisekae Set System)](https://ja.wikipedia.org/wiki/KISekae_Set_system) viewer in Qt 6.


Kisekae is short for *&quot;kisekae ningyou&quot;* (Japanese: *着せ替え人形*, meaning *&quot;dress-up dolls&quot;*).
KiSS originated from and was popular in Japanese [otaku](https://en.wikipedia.org/wiki/Otaku) and personal computer enthusiast (NEC PC-9800 series) subculture in the 1990s.

[Website of QKiSS](https://github.com/kuanyui/qkiss).
</source>
        <translation>
QKiSSは、Qt 6にて開発されましたモダンな [KiSS (Kisekae Set System)](https://ja.wikipedia.org/wiki/KISekae_Set_system) ビューアでございますわ。


Kisekaeとは「着せ替え人形」の略称でしてよ。
KiSSは1990年代、日本の[オタク](https://en.wikipedia.org/wiki/Otaku) やパソコン愛好家（NEC PC-9800シリーズ）のサブカルチャーにて生まれ、あの頃のヲタク殿方たちが熱狂なさっていたのでございますの。まあ、わたくしには到底理解できませんけれど。

[QKiSSのWebサイト](https://github.com/kuanyui/qkiss) をご覧あそばせ。
</translation>
    </message>
    <message>
        <location filename="../src/aboutdialog.cpp" line="103"/>
        <source>Information</source>
        <translation>情報でございますわ</translation>
    </message>
    <message>
        <location filename="../src/aboutdialog.cpp" line="129"/>
        <source>Programming</source>
        <translation>プログラミングでございますわ</translation>
    </message>
    <message>
        <location filename="../src/aboutdialog.cpp" line="130"/>
        <source>Icons &amp; Arts</source>
        <translation>アイコンとアートでございますわ</translation>
    </message>
    <message>
        <location filename="../src/aboutdialog.cpp" line="131"/>
        <source>Translators</source>
        <translation>翻訳をなさる方</translation>
    </message>
    <message>
        <location filename="../src/aboutdialog.cpp" line="133"/>
        <source>Authors</source>
        <translation>作者さまでいらっしゃいますの</translation>
    </message>
    <message>
        <location filename="../src/aboutdialog.cpp" line="141"/>
        <source>QKiSS is licensed under GPLv3.

QKiSS used some third-parties SVG icons from other FLOSS projects via `[tango-icon-remixer](https://github.com/kuanyui/tango-icon-remixer)`, and they are released under various licenses:</source>
        <translation>QKiSSはGPLv3ライセンスの下で提供されておりますの。

QKiSSは`[tango-icon-remixer](https://github.com/kuanyui/tango-icon-remixer)`を通じまして、他のFLOSSプロジェクトから一部のサードパーティSVGアイコンをお借りしておりまして、それらは様々なライセンスの下で公開されておりますわ。</translation>
    </message>
    <message>
        <location filename="../src/aboutdialog.cpp" line="152"/>
        <source>Licenses</source>
        <translation>ライセンスでございますの</translation>
    </message>
</context>
<context>
    <name>ArchiveFile</name>
    <message>
        <location filename="../src/archivefile.cpp" line="101"/>
        <location filename="../src/archivefile.cpp" line="125"/>
        <source>Found valid files: %1 CNF, %2 CEL, %3 KCF.</source>
        <translation>有効なファイルが見つかりましたわ：CNF %1個、CEL %2個、KCF %3個でございますの。</translation>
    </message>
    <message>
        <location filename="../src/archivefile.cpp" line="225"/>
        <source>Need to get ParsedKcf for current CNF, but this CNF has no KCF file declared. Abort.</source>
        <translation>現在のCNFに対するParsedKcfを取得いたしませんといけませんのに、このCNFにはKCFファイルの宣言がございませんの。中止いたしますわ。</translation>
    </message>
    <message>
        <location filename="../src/archivefile.cpp" line="283"/>
        <source>The CNF &apos;%1&apos; requires %2 KCF files.</source>
        <translation>CNF「%1」にはKCFファイルが%2個必要でございますの。</translation>
    </message>
    <message>
        <location filename="../src/archivefile.cpp" line="287"/>
        <source>CNF file &apos;%1&apos; requires KCF file &apos;%2&apos;, but this KCF file is not found in this archived file.</source>
        <translation>CNF「%1」はKCF「%2」を要すると申しておりますけれど、このアーカイブには当該KCFが見当たりませんの。</translation>
    </message>
    <message>
        <location filename="../src/archivefile.cpp" line="278"/>
        <source>Set current CNF failed; no CNF named &quot;%1&quot; in this archive file.</source>
        <translation>現在のCNFの設定に失敗いたしましたわ。このアーカイブには「%1」というCNFがございませんの。</translation>
    </message>
</context>
<context>
    <name>ArchiveProcessorCnfInFileSystem</name>
    <message>
        <location filename="../src/archiveprocessorcnfinfilesystem.cpp" line="66"/>
        <source>Reading a CNF file &quot;%1&quot; directly. Trying to fetch all related CNF/KCF/CEL files in the same folder...</source>
        <translation>CNFファイル「%1」を直接読み込んでおりますの。同じフォルダ内の関連するCNF/KCF/CELファイルも取得いたしておりますわ。</translation>
    </message>
</context>
<context>
    <name>ArchiveProcessorExternalLha</name>
    <message>
        <location filename="../src/archiveprocessorexternallha.cpp" line="51"/>
        <source>lha (LHa for UNIX)</source>
        <translation>lha (LHa for UNIX)</translation>
    </message>
    <message>
        <location filename="../src/archiveprocessorexternallha.cpp" line="52"/>
        <source>The de facto standard implementation of LZH.</source>
        <translation>LZH形式の事実上の標準実装でございますわ。</translation>
    </message>
    <message>
        <location filename="../src/archiveprocessorexternallha.cpp" line="53"/>
        <source>Perfect (theorically can open any LZH file)</source>
        <translation>完璧でございますの（理論上はどのLZHファイルでも開けますわ）。</translation>
    </message>
    <message>
        <location filename="../src/archiveprocessorexternallha.cpp" line="54"/>
        <source>External program; manual installation is needed.</source>
        <translation>外部プログラムでございますので、手動インストールが必要でございますの。</translation>
    </message>
    <message>
        <location filename="../src/archiveprocessorexternallha.cpp" line="55"/>
        <source>Available on most of Linux distros, also on MacPorts. Windows supporting is experimental.</source>
        <translation>ほとんどのLinuxディストリビューションとMacPortsでご利用いただけますわ。Windowsサポートは実験的でございますの。</translation>
    </message>
    <message>
        <location filename="../src/archiveprocessorexternallha.cpp" line="56"/>
        <source>&gt; 95% operations can be done in RAM instead of extracting files to disk, relatively friendly to the SSD’s lifespan.</source>
        <translation>95%以上の操作をディスクに展開せずRAM内で実行できますので、SSDの寿命にも比較的優しいのでございますわ。</translation>
    </message>
    <message>
        <location filename="../src/archiveprocessorexternallha.cpp" line="57"/>
        <source>&lt;a href=&apos;https://github.com/jca02266/lha&apos;&gt;Source Code&lt;/a&gt;&lt;/li&gt;</source>
        <translation>&lt;a href=&apos;https://github.com/jca02266/lha&apos;&gt;ソースコードでございますわ&lt;/a&gt;&lt;/li&gt;</translation>
    </message>
    <message>
        <location filename="../src/archiveprocessorexternallha.cpp" line="80"/>
        <source>External program LHA is not installed or not found.</source>
        <translation>あら、外部プログラムLHAがインストールされておりませんか、見つかりませんの。</translation>
    </message>
    <message>
        <location filename="../src/archiveprocessorexternallha.cpp" line="87"/>
        <source>Trying to extract LZH file &quot;%1&quot; via LHA...</source>
        <translation>LZHファイル「%1」を展開いたしておりますわ…。</translation>
    </message>
    <message>
        <location filename="../src/archiveprocessorexternallha.cpp" line="90"/>
        <source>Extracting LZH file &quot;%1&quot; failed. Abort.</source>
        <translation>LZHファイル「%1」の展開に失敗いたしましたの。中止いたしますわ。</translation>
    </message>
    <message>
        <location filename="../src/archiveprocessorexternallha.cpp" line="100"/>
        <source>LZH file &quot;%1&quot; is extracted successfully. Got %2 file(s) in it</source>
        <translation>あらまあ、LZHファイル「%1」の展開に成功いたしましたわ。中には%2個のファイルが含まれておりますの。</translation>
    </message>
    <message>
        <location filename="../src/archiveprocessorexternallha.cpp" line="216"/>
        <source>Failed when extracting nested LZH file &quot;%1&quot; (failed to open tmp file &quot;%2&quot;).</source>
        <translation>あら、ネストされたLZHファイル「%1」の展開に失敗いたしましたわ（一時ファイル「%2」を開けませんでしたの）。</translation>
    </message>
    <message>
        <location filename="../src/archiveprocessorexternallha.cpp" line="221"/>
        <source>Partial write occurred. Expected: %1 bytes, actual: %2 bytes</source>
        <translation>部分的な書き込みが発生いたしましたの。期待値：%1バイト、実際：%2バイトでございますわ。</translation>
    </message>
    <message>
        <location filename="../src/archiveprocessorexternallha.cpp" line="256"/>
        <source>Archive file path &apos;%1&apos; doesn&apos;t exist, abort.</source>
        <translation>アーカイブファイルパス「%1」が存在いたしませんの。中止いたしますわ。</translation>
    </message>
</context>
<context>
    <name>ArchiveProcessorExternalUnar</name>
    <message>
        <location filename="../src/archiveprocessorexternalunar.cpp" line="47"/>
        <source>Unar (Unarchiver)</source>
        <translation>Unar (Unarchiver) でございますわ</translation>
    </message>
    <message>
        <location filename="../src/archiveprocessorexternalunar.cpp" line="48"/>
        <source>An extractor specially focused on archives of old formats containing non UTF-8 encoding file names.</source>
        <translation>非UTF-8エンコーディングのファイル名を含む古い形式のアーカイブに特化した展開ツールでございますの。</translation>
    </message>
    <message>
        <location filename="../src/archiveprocessorexternalunar.cpp" line="49"/>
        <source>Pretty Good</source>
        <translation>なかなか良うございますわ</translation>
    </message>
    <message>
        <location filename="../src/archiveprocessorexternalunar.cpp" line="50"/>
        <source>External program; manual installation is needed.</source>
        <translation>外部プログラムでございますので、手動インストールが必要でございますの。</translation>
    </message>
    <message>
        <location filename="../src/archiveprocessorexternalunar.cpp" line="51"/>
        <source>Originally developed for macOS, also available on most of Linux distros. Supports Windows</source>
        <translation>もともとmacOS向けに開発されましたが、ほとんどのLinuxディストリビューションでもご利用いただけますの。Windowsもサポートしておりますわ。</translation>
    </message>
    <message>
        <location filename="../src/archiveprocessorexternalunar.cpp" line="52"/>
        <source>All extractions will do writing operations to disk; may shorten the lifespan of SSD.</source>
        <translation>すべての展開操作でディスクへの書き込みを行いますので、SSDの寿命を短くする可能性がございますの。</translation>
    </message>
    <message>
        <location filename="../src/archiveprocessorexternalunar.cpp" line="53"/>
        <source>&lt;a href=&quot;https://theunarchiver.com/command-line&quot;&gt;Official Site&lt;/a&gt;, &lt;a href=&apos;https://github.com/MacPaw/unar&apos;&gt;Source Code&lt;/a&gt;.</source>
        <translation>&lt;a href=&quot;https://theunarchiver.com/command-line&quot;&gt;公式サイトでございますわ&lt;/a&gt;、&lt;a href=&apos;https://github.com/MacPaw/unar&apos;&gt;ソースコードでございますの&lt;/a&gt;。</translation>
    </message>
    <message>
        <location filename="../src/archiveprocessorexternalunar.cpp" line="76"/>
        <source>External program Unar is not installed or not found.</source>
        <translation>あら、外部プログラムUnarがインストールされておりませんか、見つかりませんの。</translation>
    </message>
    <message>
        <location filename="../src/archiveprocessorexternalunar.cpp" line="86"/>
        <source>Extracting LZH file &quot;%1&quot; failed. Abort.</source>
        <translation>LZHファイル「%1」の展開に失敗いたしましたの。中止いたしますわ。</translation>
    </message>
    <message>
        <location filename="../src/archiveprocessorexternalunar.cpp" line="97"/>
        <source>LZH file &quot;%1&quot; is extracted successfully. Got %2 file(s) in it</source>
        <translation>あらまあ、LZHファイル「%1」の展開に成功いたしましたわ。中には%2個のファイルが含まれておりますの。</translation>
    </message>
    <message>
        <location filename="../src/archiveprocessorexternalunar.cpp" line="104"/>
        <source>Trying to extract LZH file &quot;%1&quot; via Unar...</source>
        <translation>LZHファイル「%1」を展開いたしておりますわ…。</translation>
    </message>
    <message>
        <location filename="../src/archiveprocessorexternalunar.cpp" line="125"/>
        <source>Archive file path &apos;%1&apos; doesn&apos;t exist, abort.</source>
        <translation>アーカイブファイルパス「%1」が存在いたしませんの。中止いたしますわ。</translation>
    </message>
</context>
<context>
    <name>ArchiveProcessorLibarchive</name>
    <message>
        <location filename="../src/archiveprocessorlibarchive.cpp" line="51"/>
        <source>LibArchive (built-in)</source>
        <translation>LibArchive（内蔵）でございますわ</translation>
    </message>
    <message>
        <location filename="../src/archiveprocessorlibarchive.cpp" line="52"/>
        <source>Multi-format archive and compression library.</source>
        <translation>マルチフォーマット対応のアーカイブ・圧縮ライブラリでございますの。</translation>
    </message>
    <message>
        <location filename="../src/archiveprocessorlibarchive.cpp" line="53"/>
        <source>Poor (some LZH files may be unable to be extracted, and non UTF-8 file names in LZH are totally unsupported.)</source>
        <translation>あら、一部のLZHファイルは展開できない可能性がございますし、LZH内の非UTF-8ファイル名はまったくサポートされておりませんの。</translation>
    </message>
    <message>
        <location filename="../src/archiveprocessorlibarchive.cpp" line="54"/>
        <source>Built-in in QKiSS. No extra external program is required.</source>
        <translation>QKiSSに内蔵されておりますので、追加の外部プログラムは不要でございますわ。</translation>
    </message>
    <message>
        <location filename="../src/archiveprocessorlibarchive.cpp" line="55"/>
        <source>Windows, macOS, Linux</source>
        <translation>Windows、macOS、Linux</translation>
    </message>
    <message>
        <location filename="../src/archiveprocessorlibarchive.cpp" line="56"/>
        <source>All operations can be done in RAM instead of extracting files to disk, relatively friendly to the SSD’s lifespan.</source>
        <translation>すべての操作をディスクに展開せずRAM内で実行できますので、SSDの寿命に比較的優しいのでございますわ。</translation>
    </message>
    <message>
        <location filename="../src/archiveprocessorlibarchive.cpp" line="57"/>
        <source>&lt;a href=&apos;https://github.com/libarchive/libarchive&apos;&gt;Source Code&lt;/a&gt;&lt;/li&gt;</source>
        <translation>&lt;a href=&apos;https://github.com/libarchive/libarchive&apos;&gt;ソースコードでございますの&lt;/a&gt;&lt;/li&gt;</translation>
    </message>
    <message>
        <location filename="../src/archiveprocessorlibarchive.cpp" line="77"/>
        <source>Failed to open this file, aborted.</source>
        <translation>あら、このファイルを開くことができませんでしたの。中止いたしますわ。</translation>
    </message>
    <message>
        <location filename="../src/archiveprocessorlibarchive.cpp" line="138"/>
        <source>Error when opening this archive file (%1). Aborted.</source>
        <translation>このアーカイブを開く際にエラーが発生いたしましたの（%1）。中止いたしますわ。</translation>
    </message>
    <message>
        <location filename="../src/archiveprocessorlibarchive.cpp" line="146"/>
        <source>Fatal error (%1), no more operations are possible., abort.</source>
        <translation>致命的なエラー（%1）により、これ以上の操作は叶いませんの。中止いたしますわ。</translation>
    </message>
    <message>
        <location filename="../src/archiveprocessorlibarchive.cpp" line="149"/>
        <source>Current operation cannot complete (%1), skip.</source>
        <translation>只今の操作は完遂できませんでしたの（%1）。スキップいたしますわ。</translation>
    </message>
    <message>
        <location filename="../src/archiveprocessorlibarchive.cpp" line="152"/>
        <source>Partial success (%1).</source>
        <translation>一部のみ成功いたしましたの（%1）。</translation>
    </message>
    <message>
        <location filename="../src/archiveprocessorlibarchive.cpp" line="155"/>
        <source>Retry might succeed (%1)</source>
        <translation>再試行にて成功いたしますやもしれませんわ（%1）。</translation>
    </message>
    <message>
        <location filename="../src/archiveprocessorlibarchive.cpp" line="173"/>
        <source>when reading this archive file via LibArchive (Error message: %1). Aborted.</source>
        <translation>LibArchiveでこのアーカイブを読み込み中にエラーがございましたの（エラー: %1）。中止いたしますわ。</translation>
    </message>
</context>
<context>
    <name>ConfigFile</name>
    <message>
        <location filename="../src/configfile.cpp" line="124"/>
        <source>Auto (System Language)</source>
        <translation>自動（システム言語）でございますわ</translation>
    </message>
</context>
<context>
    <name>FileSystemBrowser</name>
    <message>
        <location filename="../src/filesystembrowser.cpp" line="52"/>
        <source>File path &quot;%1&quot; is not found, abort loading this path.</source>
        <translation>あら、ファイルパス「%1」が見つかりませんの。このパスの読み込みを中止いたしますわ。</translation>
    </message>
    <message>
        <location filename="../src/filesystembrowser.cpp" line="84"/>
        <source>Invalid dir path, aborted.</source>
        <translation>無効なディレクトリパスでございますの。中止いたしましたわ。</translation>
    </message>
    <message>
        <location filename="../src/filesystembrowser.cpp" line="147"/>
        <source>File Name</source>
        <translation>ファイル名でございますわ</translation>
    </message>
    <message>
        <location filename="../src/filesystembrowser.cpp" line="147"/>
        <source>Size</source>
        <translation>サイズでございますわ</translation>
    </message>
    <message>
        <location filename="../src/filesystembrowser.cpp" line="147"/>
        <location filename="../src/filesystembrowser.cpp" line="271"/>
        <source>Patch</source>
        <translation>パッチでございますの</translation>
    </message>
    <message>
        <location filename="../src/filesystembrowser.cpp" line="185"/>
        <source>Go Back</source>
        <translation>戻りますわ</translation>
    </message>
    <message>
        <location filename="../src/filesystembrowser.cpp" line="190"/>
        <source>Go Forward</source>
        <translation>進みますの</translation>
    </message>
    <message>
        <location filename="../src/filesystembrowser.cpp" line="195"/>
        <source>Go Up</source>
        <translation>上へ参りますわ</translation>
    </message>
    <message>
        <location filename="../src/filesystembrowser.cpp" line="200"/>
        <source>Refresh</source>
        <translation>更新でございますわ</translation>
    </message>
    <message>
        <location filename="../src/filesystembrowser.cpp" line="205"/>
        <source>Home</source>
        <translation>ホームでございますの</translation>
    </message>
    <message>
        <location filename="../src/filesystembrowser.cpp" line="209"/>
        <source>Set current folder as Home</source>
        <translation>現在のフォルダをホームに設定いたしますわ</translation>
    </message>
    <message>
        <location filename="../src/filesystembrowser.cpp" line="274"/>
        <source>Click to load this file as patch</source>
        <translation>このファイルをパッチとして読み込むにはクリックなさってくださいませ</translation>
    </message>
</context>
<context>
    <name>FkissParser</name>
    <message>
        <location filename="../src/fkissparser.cpp" line="129"/>
        <source>Found a double quote mark, but cannot find another paired quote mark to form a string. SOURCE: </source>
        <translation>二重引用符を見つけましたけれど、対になるお印が見当たらず文字列が成り立ちませんの。SOURCE: </translation>
    </message>
    <message>
        <location filename="../src/fkissparser.cpp" line="147"/>
        <source>Ignore this character because it is unexpected [%1:%2]: `%3`</source>
        <translation>想定外の文字でございますゆえ無視いたしますわ [%1:%2]: `%3`</translation>
    </message>
    <message>
        <location filename="../src/fkissparser.cpp" line="284"/>
        <source>Unknown command &quot;%1&quot;, skip.</source>
        <translation>不明なコマンド「%1」でございますの。スキップいたしますわ。</translation>
    </message>
    <message>
        <location filename="../src/fkissparser.cpp" line="293"/>
        <source>[SyntaxWarning] %1   [L%2, C%3] %4</source>
        <translation>[構文警告] %1   [L%2, C%3] %4</translation>
    </message>
    <message>
        <location filename="../src/fkissparser.cpp" line="302"/>
        <source>[SyntaxError] %1   [L%2, C%3] %4</source>
        <translation>[構文エラー] %1   [L%2, C%3] %4</translation>
    </message>
</context>
<context>
    <name>GKissScene</name>
    <message>
        <location filename="../src/gkissscene.cpp" line="74"/>
        <source>No KCF found in this CNF, don&apos;t draw set background.</source>
        <translation>このCNFにはKCFが見当たりませんので、背景はお描きいたしませんわ。</translation>
    </message>
    <message>
        <location filename="../src/gkissscene.cpp" line="81"/>
        <source>No palette in this KCF. Abort.</source>
        <translation>このKCFにはパレットがございませんの。中止いたしますわ。</translation>
    </message>
    <message>
        <location filename="../src/gkissscene.cpp" line="92"/>
        <source>In this CNF, setNo %1 specifies its default paletteNo as %2, but the first KCF contains only %3 palettes. Fallback to paletteNo 0 in the first KCF.</source>
        <translation>このCNFでは setNo %1 の既定 paletteNo が %2 と定められておりますけれど、最初のKCFにはパレットが%3個しかございませんの。最初のKCFの paletteNo 0 に差し戻してさしあげますわ。</translation>
    </message>
    <message>
        <location filename="../src/gkissscene.cpp" line="100"/>
        <source>WTF? In CNF&apos;s set definition, not found palette in first KCF of this CNF. Abort.</source>
        <translation>まあ…… CNF の set 定義において、このCNFの最初の KCF にパレットが見当たりませんの。中止いたしますわ。</translation>
    </message>
    <message>
        <location filename="../src/gkissscene.cpp" line="550"/>
        <source>[%1] Unfix (&amp;U)</source>
        <translation>[%1] 固定を解除いたしますわ(&amp;U)</translation>
    </message>
    <message>
        <location filename="../src/gkissscene.cpp" line="552"/>
        <source>Reset Object (&amp;R)</source>
        <translation>オブジェクトをリセットいたしますの(&amp;R)</translation>
    </message>
    <message>
        <location filename="../src/gkissscene.cpp" line="554"/>
        <source>Information</source>
        <translation>情報でございますわ</translation>
    </message>
    <message>
        <location filename="../src/gkissscene.cpp" line="561"/>
        <source>Click to copy filename to clipboard.</source>
        <translation>ファイル名をクリップボードにコピーなさるにはクリックしてくださいませ。</translation>
    </message>
</context>
<context>
    <name>GKissView</name>
    <message>
        <location filename="../src/gkissview.cpp" line="60"/>
        <source>No CNF file found in this archive file.</source>
        <translation>このアーカイブにはCNFが見当たりませんの。</translation>
    </message>
    <message>
        <location filename="../src/gkissview.cpp" line="91"/>
        <source>No CNF file named &quot;%1&quot; found in this archive file.</source>
        <translation>このアーカイブに「%1」というCNFはございませんの。</translation>
    </message>
    <message>
        <location filename="../src/gkissview.cpp" line="115"/>
        <location filename="../src/gkissview.cpp" line="121"/>
        <source>Only %1 scenes in this view but you ask for sceneIndex %2</source>
        <translation>本ビューには%1件のシーンしかございませんのに、sceneIndex %2 が指定されておりますわ。</translation>
    </message>
    <message>
        <location filename="../src/gkissview.cpp" line="190"/>
        <source>CNF requires a CEL &quot;%1&quot;, but not found in this archived file: </source>
        <translation>CNF は CEL「%1」を要しておりますけれど、このアーカイブには見当たりませんの： </translation>
    </message>
    <message>
        <location filename="../src/gkissview.cpp" line="199"/>
        <source>In CNF &quot;%1&quot;, the CEL &quot;%2&quot; requires KCF index %3, but no KCF file available in this CNF.</source>
        <translation>CNF「%1」において、CEL「%2」は KCF インデックス %3 を要しますのに、このCNFにはKCFがございませんの。</translation>
    </message>
    <message>
        <location filename="../src/gkissview.cpp" line="224"/>
        <source>[Benchmark] Creating all graphics items in %1 scene(s) cost %2 ms</source>
        <translation>[ベンチマーク] %1シーンにおける全グラフィック項目の作成に%2 ms でございますわ</translation>
    </message>
    <message>
        <location filename="../src/gkissview.cpp" line="226"/>
        <source>No scene found. Aborted</source>
        <translation>シーンが見当たりませんの。中止いたしましたわ。</translation>
    </message>
</context>
<context>
    <name>InnerFilesBrowser</name>
    <message>
        <location filename="../src/innerfilesbrowser.cpp" line="35"/>
        <source>CNF files</source>
        <translation>CNF文書群</translation>
    </message>
    <message>
        <location filename="../src/innerfilesbrowser.cpp" line="39"/>
        <source>Document files</source>
        <translation>文書典籍</translation>
    </message>
</context>
<context>
    <name>ItemInspector</name>
    <message>
        <location filename="../src/iteminspector.cpp" line="47"/>
        <source>CEL File</source>
        <translation>CEL文書</translation>
    </message>
    <message>
        <location filename="../src/iteminspector.cpp" line="48"/>
        <source>Object ID</source>
        <translation>対象識別子</translation>
    </message>
    <message>
        <location filename="../src/iteminspector.cpp" line="49"/>
        <source>CEL Position</source>
        <translation>CEL座標</translation>
    </message>
    <message>
        <location filename="../src/iteminspector.cpp" line="50"/>
        <source>Fix</source>
        <translation>定着</translation>
    </message>
    <message>
        <location filename="../src/iteminspector.cpp" line="51"/>
        <source>CEL Size</source>
        <translation>CEL寸法</translation>
    </message>
    <message>
        <location filename="../src/iteminspector.cpp" line="52"/>
        <source>CEL Comment</source>
        <translation>CEL注記</translation>
    </message>
    <message>
        <location filename="../src/iteminspector.cpp" line="53"/>
        <source>CEL Preview</source>
        <translation>CEL御覧</translation>
    </message>
    <message>
        <location filename="../src/iteminspector.cpp" line="54"/>
        <source>CEL Siblings</source>
        <translation>CEL姉妹要素</translation>
    </message>
</context>
<context>
    <name>LogViewer</name>
    <message>
        <location filename="../src/logviewer.cpp" line="38"/>
        <source>Debug</source>
        <translation>調整記録</translation>
    </message>
    <message>
        <location filename="../src/logviewer.cpp" line="43"/>
        <source>Info</source>
        <translation>告知</translation>
    </message>
    <message>
        <location filename="../src/logviewer.cpp" line="48"/>
        <source>Warning</source>
        <translation>警告</translation>
    </message>
    <message>
        <location filename="../src/logviewer.cpp" line="53"/>
        <source>Error</source>
        <translation>過誤</translation>
    </message>
    <message>
        <location filename="../src/logviewer.cpp" line="58"/>
        <source>Fatal</source>
        <translation>致命断</translation>
    </message>
    <message>
        <location filename="../src/logviewer.cpp" line="76"/>
        <source>Severity</source>
        <translation>重み</translation>
    </message>
    <message>
        <location filename="../src/logviewer.cpp" line="76"/>
        <source>Message</source>
        <translation>御言</translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location filename="../src/mainwindow.ui" line="32"/>
        <source>&amp;File</source>
        <translation>御文書(&amp;F)</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.ui" line="59"/>
        <source>&amp;View</source>
        <translation>御覧(&amp;V)</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.ui" line="67"/>
        <source>&amp;Window</source>
        <translation>御窓(&amp;W)</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.ui" line="72"/>
        <source>&amp;Settings</source>
        <translation>御調え(&amp;S)</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.ui" line="36"/>
        <source>Open R&amp;ecent Directory</source>
        <translation>近時の書庫を開きますわ(&amp;E)</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.ui" line="42"/>
        <source>Open &amp;Recent File</source>
        <translation>近時の文書を開きますの(&amp;R)</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.ui" line="76"/>
        <source>&amp;Language</source>
        <translation>御言葉(&amp;L)</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.ui" line="84"/>
        <source>&amp;Encoding</source>
        <translation>御文字綴り(&amp;E)</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.ui" line="92"/>
        <source>&amp;Case Sensitivity</source>
        <translation>大文字小文字の御取扱い(&amp;C)</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.ui" line="103"/>
        <source>&amp;Help</source>
        <translation>御助け(&amp;H)</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.ui" line="122"/>
        <source>File System Browser</source>
        <translation>文書体系御覧</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.ui" line="131"/>
        <source>Inner File Selector</source>
        <translation>内部文書選び</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.ui" line="149"/>
        <source>Logs</source>
        <translation>記録簿ですわ</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.ui" line="161"/>
        <source>CNF Code Viewer</source>
        <translation>CNF原始コード御覧</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.ui" line="170"/>
        <source>Document Viewer</source>
        <translation>文書御覧</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.ui" line="179"/>
        <source>Item Inspector</source>
        <translation>品目検分</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.ui" line="191"/>
        <source>&amp;Open File</source>
        <translation>文書を開きますわ(&amp;O)</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.ui" line="199"/>
        <source>Open &amp;Directory</source>
        <translation>書庫を開きますの(&amp;D)</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.ui" line="207"/>
        <source>&amp;Quit</source>
        <translation>御退出(&amp;Q)</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.ui" line="218"/>
        <source>Zoom &amp;Out</source>
        <translation>縮小いたしますわ(&amp;O)</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.ui" line="226"/>
        <source>Zoom &amp;In</source>
        <translation>拡大いたしますの(&amp;I)</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.ui" line="234"/>
        <source>&amp;Reset Scale</source>
        <translation>尺度を元へ戻しますわ(&amp;R)</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.ui" line="239"/>
        <location filename="../src/mainwindow.ui" line="244"/>
        <location filename="../src/mainwindow.cpp" line="658"/>
        <location filename="../src/mainwindow.cpp" line="675"/>
        <source>Clear All</source>
        <translation>すべて消去いたしますわ</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.ui" line="252"/>
        <source>&amp;Preferences</source>
        <translation>御調え(&amp;P)</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.ui" line="255"/>
        <source>Preferences</source>
        <translation>御調え</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.ui" line="266"/>
        <source>&amp;About</source>
        <translation>御案内(&amp;A)</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.ui" line="269"/>
        <source>About QKiSS</source>
        <translation>QKiSSの御案内</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="76"/>
        <source>Fix: %1, CEL Name: %2, Comment: %3</source>
        <translation>固定：%1、CEL名：%2、注記：%3</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="165"/>
        <source>Start to parse CNF / KCF / CEL files...</source>
        <translation>CNF / KCF / CEL文書の解析を開始いたしますわ…</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="171"/>
        <source>&quot;%1&quot; is not a valid KiSS patch file. Aborted.</source>
        <translation>「%1」は正規のKiSSパッチ文書ではございませんわ。処理を中止いたしましたの</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="178"/>
        <source>&quot;%1&quot;is not a valid KiSS file. Aborted.</source>
        <translation>「%1」は正規のKiSS文書ではございませんの。処理を中止いたしましたわ</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="186"/>
        <source>Parsing finished. Now rendering scenes...</source>
        <translation>解析が完了いたしましたわ。これより場景を描画いたしますの</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="199"/>
        <source>KiSS files rendered successfully: %1</source>
        <translation>KiSS文書の描画が無事に完了いたしましたわ：%1</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="213"/>
        <location filename="../src/mainwindow.cpp" line="219"/>
        <location filename="../src/mainwindow.cpp" line="220"/>
        <source>Scene Sets</source>
        <translation>御場景集</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="221"/>
        <source>Switch scene set (F1/F2)</source>
        <translation>御場景集を切り替えますわ（F1/F2）</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="244"/>
        <location filename="../src/mainwindow.cpp" line="250"/>
        <location filename="../src/mainwindow.cpp" line="251"/>
        <source>Palette Sets</source>
        <translation>御彩盤組</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="252"/>
        <source>Switch palette set (F3/F4)</source>
        <translation>御彩盤組を切り替えますわ（F3/F4）</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="393"/>
        <location filename="../src/mainwindow.cpp" line="430"/>
        <source>Files</source>
        <translation>文書ですの</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="401"/>
        <location filename="../src/mainwindow.cpp" line="431"/>
        <source>Zooming Tools</source>
        <translation>拡縮具でしてよ</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="414"/>
        <location filename="../src/mainwindow.cpp" line="432"/>
        <source>Windows</source>
        <translation>御窓ですわ</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="524"/>
        <source>QKiSS ready.</source>
        <translation>QKiSS、準備整いましたわ</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="528"/>
        <source>Ctrl+Drag: Force split CEL from Object</source>
        <translation>Ctrl+ドラッグ：オブジェクトからCELを強制的に分離いたしますの</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="533"/>
        <source>Shift+Drag: Disable FKiSS events</source>
        <translation>Shift+ドラッグ：FKiSSイベントを無効にいたしますわ</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="538"/>
        <source>Middle Click: Unfix</source>
        <translation>中クリック：固定を解除いたしますの</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="543"/>
        <source>Right Click: More Actions</source>
        <translation>右クリック：その他の操作でしてよ</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="685"/>
        <source>LZH Archive Files (*.lzh)</source>
        <translation>LZH アーカイブファイルですわ (*.lzh)</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="687"/>
        <source>Open Compressed KiSS File</source>
        <translation>圧縮KiSS文書を開きますわ</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="702"/>
        <source>Open Directory</source>
        <translation>書庫を開きますの</translation>
    </message>
</context>
<context>
    <name>ParsedCel</name>
    <message>
        <location filename="../src/parsedcel.cpp" line="79"/>
        <source>Invalid m_bitsPerPixel (4 or 8 or 32): %1</source>
        <translation>m_bitsPerPixel が不正でございますわ（4・8・32 のみ）: %1</translation>
    </message>
    <message>
        <location filename="../src/parsedcel.cpp" line="82"/>
        <source>&quot;%1&quot; claims it is a palette-based CEL file, but it said each pixel is %2 bits (expected: 32 bits), this is contradicted. Assume it is a palette-based CEL.</source>
        <translation>「%1」はパレット方式のCELと称しておりますけれど、各画素が%2ビット（想定は32ビット）と申しており矛盾いたしますの。パレット方式のCELと見なして進めますわ。</translation>
    </message>
</context>
<context>
    <name>ParsedCnf</name>
    <message>
        <location filename="../src/parsedcnf.cpp" line="215"/>
        <source>Wrong playFieldSize format.</source>
        <translation>playFieldSize のお書式が正しくございませんの。</translation>
    </message>
    <message>
        <location filename="../src/parsedcnf.cpp" line="221"/>
        <source>Wrong KCF file name format.</source>
        <translation>KCF ファイル名のお書式が正しくございませんの。</translation>
    </message>
    <message>
        <location filename="../src/parsedcnf.cpp" line="227"/>
        <source>Wrong outsideColor format.</source>
        <translation>outsideColor のお書式が正しくございませんの。</translation>
    </message>
    <message>
        <location filename="../src/parsedcnf.cpp" line="245"/>
        <source>Wrong CEL format.</source>
        <translation>CEL のお書式が正しくございませんの。</translation>
    </message>
    <message>
        <location filename="../src/parsedcnf.cpp" line="286"/>
        <source>Wrong set line format.</source>
        <translation>set 行のお書式が正しくございませんの。</translation>
    </message>
    <message>
        <location filename="../src/parsedcnf.cpp" line="331"/>
        <source>Wrong fkiss prefix `:@`, but for tolerance of legacy accepts it =&gt;  </source>
        <translation>fkiss の接頭辞 `:@` は正しくございませんけれど、旧来互換のため受け入れてさしあげますわ =&gt;  </translation>
    </message>
    <message>
        <location filename="../src/parsedcnf.cpp" line="356"/>
        <source>Wrong objPos format</source>
        <translation>objPos のお書式が正しくございませんの。</translation>
    </message>
</context>
<context>
    <name>ParsedKcf</name>
    <message>
        <location filename="../src/parsedkcf.cpp" line="42"/>
        <source>Not a palette file mark!</source>
        <translation>パレットファイルの印ではございませんわ！</translation>
    </message>
    <message>
        <location filename="../src/parsedkcf.cpp" line="46"/>
        <source>Invalid bitsPerColor (12 or 24): %1</source>
        <translation>bitsPerColor が不正でございますの（12 または 24）: %1</translation>
    </message>
    <message>
        <location filename="../src/parsedkcf.cpp" line="50"/>
        <source>Invalid colorAmountsInOnePalette (1 ~ 256): %1</source>
        <translation>colorAmountsInOnePalette が不正でございますわ（1 ～ 256）: %1</translation>
    </message>
    <message>
        <location filename="../src/parsedkcf.cpp" line="60"/>
        <location filename="../src/parsedkcf.cpp" line="78"/>
        <location filename="../src/parsedkcf.cpp" line="112"/>
        <source>Out of range palatte colors definition, byte offset: %1</source>
        <translation>パレット色の定義が範囲外でございますの。バイトオフセット: %1</translation>
    </message>
    <message>
        <location filename="../src/parsedkcf.cpp" line="90"/>
        <source>What is this? Unknown KCF bitsPerColor: %1</source>
        <translation>まあ？ 見知らぬ KCF bitsPerColor でございますわ: %1</translation>
    </message>
    <message>
        <location filename="../src/parsedkcf.cpp" line="142"/>
        <source>This KCF file contains no valid palette, but you try to get palette from it. Abort.</source>
        <translation>このKCFには有効なパレットがございませんのに、パレットを求めようとなさいますの？ 中止いたしますわ。</translation>
    </message>
    <message>
        <location filename="../src/parsedkcf.cpp" line="146"/>
        <source>paletteIdx (%1, note this is counted from 0) is out of actual paletteAmount (%2), use index 0 as fallback</source>
        <translation>paletteIdx（%1、0始まりにご留意くださいませ）が実際の paletteAmount（%2）を超えておりますの。代替として 0 を用いますわ。</translation>
    </message>
</context>
<context>
    <name>PreferenceDialog</name>
    <message>
        <location filename="../src/preferencedialog.cpp" line="40"/>
        <source>Preferences</source>
        <translation>御調え</translation>
    </message>
    <message>
        <location filename="../src/preferencedialog.cpp" line="63"/>
        <source>LZH Extractor Backend</source>
        <translation>LZH展開の後仕えですの</translation>
    </message>
    <message>
        <location filename="../src/preferencedialog.cpp" line="144"/>
        <source>Backend</source>
        <translation>後仕え</translation>
    </message>
    <message>
        <location filename="../src/preferencedialog.cpp" line="149"/>
        <source>Status</source>
        <translation>御様子</translation>
    </message>
    <message>
        <location filename="../src/preferencedialog.cpp" line="165"/>
        <source>Name</source>
        <translation>御名</translation>
    </message>
    <message>
        <location filename="../src/preferencedialog.cpp" line="166"/>
        <source>Summary</source>
        <translation>御概要</translation>
    </message>
    <message>
        <location filename="../src/preferencedialog.cpp" line="167"/>
        <source>LZH Compatibility</source>
        <translation>LZH互換性でしてよ</translation>
    </message>
    <message>
        <location filename="../src/preferencedialog.cpp" line="168"/>
        <source>Installation</source>
        <translation>御導入</translation>
    </message>
    <message>
        <location filename="../src/preferencedialog.cpp" line="169"/>
        <source>OS Support</source>
        <translation>OS御支援</translation>
    </message>
    <message>
        <location filename="../src/preferencedialog.cpp" line="170"/>
        <source>Disk Operation</source>
        <translation>御盤操作</translation>
    </message>
    <message>
        <location filename="../src/preferencedialog.cpp" line="171"/>
        <source>Website</source>
        <translation>御サイト</translation>
    </message>
    <message>
        <location filename="../src/preferencedialog.cpp" line="173"/>
        <source>Backend Information</source>
        <translation>後仕え情報ですの</translation>
    </message>
    <message>
        <location filename="../src/preferencedialog.cpp" line="210"/>
        <source>Installed</source>
        <translation>御導入済み</translation>
    </message>
    <message>
        <location filename="../src/preferencedialog.cpp" line="213"/>
        <source>Not Found / Not Installed</source>
        <translation>見当たりません / 未導入ですわ</translation>
    </message>
</context>
<context>
    <name>QKiSS</name>
    <message>
        <location filename="../src/enums.h" line="58"/>
        <source>ASCII</source>
        <translation>ASCII（どうやら古式文字の御符号でしてよ。異国の原始人が用いておりましたの）</translation>
    </message>
    <message>
        <location filename="../src/enums.h" line="59"/>
        <source>UTF-8</source>
        <translation>UTF-8（万国共通の御綴り様式ですわ）</translation>
    </message>
    <message>
        <location filename="../src/enums.h" line="60"/>
        <source>Shift JIS</source>
        <translation>Shift JIS（和文文字の由緒正しき御体系でしてよ。推奨される正統の様式ですわ）</translation>
    </message>
    <message>
        <location filename="../src/enums.h" line="61"/>
        <source>EUC-JP</source>
        <translation>EUC-JP（正統なるUNIX御用達の御体系ですの）</translation>
    </message>
    <message>
        <location filename="../src/enums.h" line="66"/>
        <source>Case Insensitive</source>
        <translation>大文字小文字をお構いなしですの</translation>
    </message>
    <message>
        <location filename="../src/enums.h" line="67"/>
        <source>Case Sensitive</source>
        <translation>大文字小文字をきっちり区別いたしますわ</translation>
    </message>
</context>
</TS>
