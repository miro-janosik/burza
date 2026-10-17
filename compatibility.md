# Burza — platform compatibility

What Burza is expected to build and run on, which branch serves which target,
and which C++ standard each one is pinned to.

Three tiers, oldest first. Each tier is defined by the oldest operating system
it must serve, because that is what caps the GTK version, and the GTK version
then fixes everything else — the UI designer, the compiler and the C++
standard.

Read this before the table: **Tiers 0 and 1 are the same code on the same
toolkit.** Burza has only ever had one GTK 2 codebase — gtkmm 2.4 with
libglademm — and it works on XP, Windows 7 and Ubuntu 18.04 alike. The tiers
are not two technologies; they are two *support levels* over one build:

- **Tier 0** is XP. No longer supported or tested. Kept on record.
- **Tier 1** is Windows 7 and Ubuntu 18.04. Supported, bugfix-only, on the
  `Gtk2` branch.
- **Tier 2** is the GTK 4 rewrite, on `Cambalache`, heading for `main`.

| | Tier 0 | Tier 1 | Tier 2 |
|---|---|---|---|
| **Oldest OS** | Windows XP SP3 | Windows 7 SP1 · Ubuntu 18.04 | Windows 10 · Ubuntu 22.04 |
| **Toolkit** | gtkmm 2.4 / GTK+ 2.2x | gtkmm 2.4 / GTK+ 2.2x | gtkmm 4.x / GTK 4.6+ |
| **UI designer** | Glade 3.8 (`.glade`) | Glade 3.8 (`.glade`) | Cambalache (`.ui` + `.cmb`) |
| **C++ standard** | C++11 | C++11 | C++20 |
| **Branch** | `master` (frozen) | `Gtk2` | `Cambalache` → `main` |
| **Status** | in history | supported | in progress |

There is **no GTK 3 tier**. GTK 3 is not used, not planned, and has never been
implemented — see *GTK 3: tried once, abandoned* below for the one historical
attempt, so it is not mistaken for existing work.

---

## Tier 0 — Windows XP · in history

The oldest system the gtkmm 2.4 build reaches. Technically identical to
Tier 1 — same branch lineage, same toolkit, same C++ standard — so this tier is
a statement about *support*, not about code.

What makes XP its own tier is the ceiling: **GTK 2 is as far as XP goes.**
GTK 3 dropped XP early in the 3.x series and has required Vista or newer ever
since. Tier 1 has the same toolkit by choice; Tier 0 has it by force.

Not tested and not supported. Not lost either: `master` holds the complete,
last known-good source, and `run/` holds a working Windows runtime.

| | |
|---|---|
| Toolkit | gtkmm 2.4 API series, libglademm 2.4 |
| GTK version | GTK+ 2.24 is the last GTK 2 release (Ubuntu 18.04 carries 2.24.32); the shipped Windows runtime in `run/` was built from the `gtkmm-win32-devel-2.22.0` bundle in `external/` |
| UI files | `.glade` (libglade format, `<glade-interface>`, `interface-requires gtk+ 2.16`) |
| Windows toolchain | MSVC v141_xp (VS2017 XP toolset) — the last that targets XP |
| C++ standard | **C++11** — what `master` sets; C++14 mostly works; C++17 only partially (the v141_xp CRT lacks parts of the C++17 library) |

**If an XP machine needs to run Burza, it does not need this tier reopened.**
The gtkmm 2 Windows build already exists and works — `run/` holds the whole
runtime (GTK 2 DLLs, `icudt48.dll`, vcredist). Shipping that frozen binary
costs nothing.

A note on the version numbering, since it reads oddly: `gtkmm-2.4` is the
*API series* name and stayed fixed across every gtkmm 2.x release, so the
`PKG_CHECK_MODULES(GTKMM REQUIRED gtkmm-2.4)` on `master` says nothing about
which GTK 2 you get — that comes from the distribution or the Windows bundle.

---

## Tier 1 — Windows 7 · Ubuntu 18.04 — gtkmm 2.4 · branch `Gtk2`

**The supported legacy tier.** These machines run Burza today, on the gtkmm 2.4
build, and that is what they will keep running. No toolkit change is planned.

Windows 7 runs the GTK 2 runtime in `run/` as-is, and Ubuntu 18.04 still
packages `libgtkmm-2.4-dev` (2.24.5) and `libgtk2.0-dev` (2.24.32), so nothing
here needs porting to keep working.

| | |
|---|---|
| Toolkit | gtkmm 2.4 / GTK+ 2.2x, libglademm 2.4 — same as Tier 0 |
| UI files | `.glade` (libglade format) |
| Linux compiler | GCC 7.5 (Ubuntu 18.04 default) |
| Windows compiler | MSVC v142 (VS2019) targeting Win7, or MinGW-w64 |
| C++ standard | **C++11** |

### Branch

`Gtk2`, to be created from `master`. It does **not exist yet**:

```
git switch -c Gtk2 master
```

Because the toolkit is unchanged, this is a true branch-and-go — unlike a GTK 3
or GTK 4 move, there is no port to land first. The branch exists to give the
legacy tier a place to receive bugfixes without disturbing `master` as the
historical record, and without touching `Cambalache`.

Keep it **bugfix-only**. The moment features start landing on both `Gtk2` and
`Cambalache`, there are two codebases to maintain, which is the one outcome the
tier model is meant to avoid.

### C++ standard

C++11, because that is what `master` sets and there is no reason to move it.

Worth knowing, should it ever come up: the toolkit does not constrain this.
gtkmm 2.4 builds fine under newer standards, and both of this tier's compilers
reach C++17 (GCC 7.5 on Ubuntu 18.04 for the language, GCC 8 via the toolchain
PPA for the full library; MSVC v142 for Windows 7). So C++11 here is a
*decision not to churn*, not a limit. `std::format` remains unavailable either
way — that needs GCC 13.

### GTK 3: tried once, abandoned

So this is not rediscovered as unfinished work: a GTK 3 move was attempted and
reverted, years ago, and it never got beyond the build file.

| | |
|---|---|
| `385a188` | 2014-10-22 — flipped `pkg_check_modules(GTKMM REQUIRED gtkmm-2.4)` to `gtkmm-3.0` |
| `246c7a7` | 2016-02-29 — flipped it back to `gtkmm-2.4`, commit titled "compilable on RasPi 2" |

It was a one-line dependency bump, not a port:

- No commit in the repository's entire history touches anything under `src/`
  for `gtkmm-3.0`. `bdialogs.cpp` was never adapted.
- `libglademm-2.4` stayed in place, and libglademm is GTK 2-only, so
  gtkmm-3.0 beside libglademm-2.4 could not have worked properly.

Both commits are in `master`'s history, so `master` went 2.4 → 3.0 → 2.4. The
current state is 2.4, and that is deliberate.

For reference, had GTK 3 ever been wanted, the measured cost in `master`'s
`src/bdialogs.cpp` is 9 `Gnome::Glade::Xml` sites plus 8 `get_widget_derived`
sites — 17 places tied to libglademm — plus converting the `.glade` file to
GtkBuilder `.ui`. `Gdk::Color` (4 sites) and `Gtk::Stock` (2) are deprecated in
gtkmm 3 but still compile. Not planned.

---

## Tier 2 — GTK 4 + Cambalache · branch `Cambalache` → `main`

The current development tier, and the only one being actively developed. Merges
to `main` when ready.

| | |
|---|---|
| Toolkit | gtkmm 4.0+ / GTK 4.6+ (Ubuntu 25.x packages gtkmm 4.20) |
| UI files | `resources/burza.ui` + `resources/burza.cmb`, symlinked into `run_linux/data/` |
| Linux | Ubuntu 22.04+ — GTK 4 needs glib ≥ 2.66, and 18.04 has 2.56 |
| Windows | Windows 10+; MSYS2 `mingw-w64-x86_64-gtkmm-4.0`, or vcpkg |
| Compiler | GCC 13+ · Clang 17+ (libc++) · MSVC 19.29+ / VS2022 |
| C++ standard | **C++20** |

C++20 is required today for exactly one reason: `std::format`. Everything else
in the tree is C++14 or C++17. See below.

Note that the deprecated-in-GTK-4.10 widgets this code leans on —
`Gtk::TreeView`, `Gtk::ListStore`, `Gtk::CellRendererToggle`,
`Gtk::TreeViewColumn` — still compile on gtkmm 4.20 with deprecation warnings.
They will need replacing with `Gtk::ColumnView` eventually, but that is a
separate piece of work from this tier's definition.

---

## C++ standard per tier

| Standard | Tier | Compilers | Do not use |
|---|---|---|---|
| C++11 | 0 and 1 | MSVC v141_xp (XP), v142 (Win7), GCC 4.8+ | generic lambdas, `std::make_unique`, chrono literals, `std::optional`, `std::string_view` |
| C++20 | 2 | GCC 13+, Clang 17+, VS2022 | — |

There is no C++17 tier, because there is no GTK 3 tier. The gap between C++11
and C++20 is wide, and it is the main thing that makes code sharing between the
legacy and GTK 4 branches awkward — see below.

Set it per build; do not hardcode it. The CMake option on `Cambalache`:

```
cmake -S burza_kdev_4 -B build -DBURZA_CXX_STANDARD=11
```

`BURZA_CXX_STANDARD` accepts 11, 14, 17 or 20 and defaults to 20.
`burza_kdev_4/CMakeLists.txt` also keeps `cmake_minimum_required(VERSION 3.10)`
and uses no command newer than 3.10, so the build system itself still
configures on Ubuntu 18.04 (cmake 3.10.2) — which is a different thing from the
program building there. Note that `master`, and therefore `Gtk2`, still carries
the old CMakeLists with a hardcoded `-std=c++11` and
`cmake_minimum_required(VERSION 2.8)`; the latter no longer configures on
CMake 4.x, so that is worth fixing on `Gtk2` even though nothing else changes.

### The one real portability blocker

`std::format` is used in **four places**, and it is the only C++20 feature in
the entire codebase:

- `src/datapostgrespqxx.cpp:114` and `:117`
- `src/xmltool.cpp:21` and `:24`

The project's own `ConvFuncs::FormatStdStr` already does the same job. Replacing
those four calls drops Tier 2 to C++17 and widens its compiler range
considerably — and it is a prerequisite for any code sharing with `Gtk2`, which
is C++11.

Worth doing on `Cambalache` regardless of the tier question: it is four lines
and it removes the project's only hard dependency on a very recent compiler.

### Sharing code between `Gtk2` and `Cambalache`

Tempting, because the domain layer is almost free of GTK:

| Layer | GTK symbol uses |
|---|---|
| `bdialogs.cpp` / `.h` | 328 |
| `LoadingMultipleFiles`, `bdfilehelper`, `bdhelper`, `EditMessageBox` | ~75 |
| `burza.cpp` / `.h`, `burzamain.cpp` | 18 |
| `log.cpp` / `.h` | 24 — all inside `#ifdef LOG_INTO_GTKMM_CONTROLS`, which is never defined |
| **everything else** | **0** |

"Everything else" is `dataaccess`, `datapostgrespqxx`, `datatextfile`,
`cacheddatastorage`, `fees`, `recipees`, `pdfwrap`, `excelwrap`, `settings`,
`xmltool`, `convfncs`, `convfile`, `stringencoding`, `itemsearch`, `licence`,
`format` and `generator` — the entire business layer. The only coupling from
core back to the GUI is **three** calls to
`BurzaDialogs::DisplayErrorMessageBox` (`src/errhandler.cpp`,
`src/settings.cpp`) plus one `ChangeGtkmmAppFontSize`.

So a shared `burza_core` static library is technically within reach. But note
the cost before committing to it: the shared core would have to compile at
**C++11**, the lower of the two tiers, which means Tier 2 gives up C++14/17
features in the core as well, not just `std::format`.

Given that `Gtk2` is meant to be bugfix-only, **cherry-picking core fixes
between the two branches is probably the cheaper answer** than extracting a
shared library and constraining Tier 2 to C++11. Revisit this only if Tier 1
starts receiving real feature work.

---

## UI design tooling

### Glade for Tiers 0 and 1, Cambalache for Tier 2 — and they do not overlap

The GTK 2 tiers and the GTK 4 tier cannot share a designer, and the reason is
worth being explicit about:

- **Cambalache cannot author GTK 2 at all.** It targets `gtk-4.0` and
  `gtk-3.0` only. There is no GTK 2 mode. So it is of no use to `Gtk2`.
- **Glade cannot author GTK 4 at all**, and modern Glade cannot author GTK 2
  either: Glade 3.8.x was the last release that edits the libglade
  `<glade-interface>` format. From 3.10 onwards Glade is GTK 3 and GtkBuilder
  only.

This produces a practical problem for Tier 1 that is easy to miss: **Ubuntu
18.04 packages Glade 3.22, which cannot open `burza.glade`.** Editing the
Tier 1 interface needs a Glade 3.8 build, which is not in any current
distribution.

In practice this means the Tier 1 UI is effectively frozen, which is consistent
with the tier being bugfix-only. If a Tier 1 UI change does become necessary,
the options are a preserved Glade 3.8 build (or VM/container), or hand-editing
the XML, which is plain enough for small changes.

### Cambalache's own requirements

Cambalache is a *development tool*, so these constrain the developer's machine,
never the deployment target.

Recent releases (0.99.x):

| Component | Minimum | Required? |
|---|---|---|
| GTK 4 | 4.18 | Yes |
| libadwaita | 1.7 | Yes |
| GtkSourceView 5 | 5.16 | Yes |
| PyGObject | 3.52 | Yes |
| Casilda (live preview renderer) | 1.0 | Yes |
| Python 3 | 3.11 – 3.14 | Yes |
| libxml2 (+ lxml) | 2.9 | Yes |
| GTK 3 | 3.24 | Only to design GTK 3 UIs |
| libhandy | 1.8 | Optional |
| WebKitGTK | 2.48 | Optional |

Which hosts can run it:

| System | Can run Cambalache? |
|---|---|
| Ubuntu 18.04 | **No** — ships GTK 3.22, no GTK 4 at all |
| Windows 7 | **No** — modern GTK 4 and libadwaita builds do not support it |
| Ubuntu 24.04 | Not the newest releases — has GTK 4.14 and libadwaita 1.5, so use Flatpak or an older Cambalache |
| Recent Ubuntu, Fedora, Arch — any distro with GTK 4.18+ | Yes |

On an older host the practical route is the **Flatpak** build, which bundles its
own GNOME runtime so the host's GTK version stops mattering:

```
flatpak install flathub ar.xjuan.Cambalache
flatpak run ar.xjuan.Cambalache resources/burza.cmb
```

Flatpak itself still needs a reasonably modern base; Ubuntu 18.04 is borderline.

Older Cambalache releases asked for much less — an early spec file lists GTK 4.4
and libadwaita 1.1.2 — but those versions are unmaintained and are worth
considering only when genuinely stuck on an old distribution.

Source for the version figures: the Gentoo ebuild dependency list
(<https://gpo.zugaina.org/dev-util/cambalache/Dep>) and the openSUSE spec file.

---

## Branch strategy

```
master ───────────┬────────────────▶  Tier 0  XP — frozen, in history
 gtkmm 2.4        │
 -std=c++11       └┄┄ Gtk2 ┄┄┄┄┄┄▶  Tier 1  Win7 / Ubuntu 18.04 — NOT YET CREATED
                                              same toolkit, so no port needed

Cambalache ────────────────────────▶  Tier 2  GTK 4 — merges to main when ready
 gtkmm 4.x
 -std=c++20
```

(`┄┄` = planned, does not exist yet.)

- **`master`** — the single gtkmm 2.4 / `-std=c++11` codebase, and the Tier 0
  record. Frozen once `Gtk2` is cut.
- **`Gtk2`** — to be branched from `master`, unchanged in toolkit. Receives
  Tier 1 bugfixes. Branch-and-go: no port required.
- **`Cambalache`** — current GTK 4 work. Becomes `main` when ready.

No GTK 3 branch exists or is planned.

---

## Alternative on record

The research behind this document ended on a different recommendation worth
keeping visible: **Qt 5.15 with Qt Designer**, on the grounds that Windows 7 is
precisely what makes the GTK options awkward — Qt 5.15 supports Windows 7 and
Ubuntu 18.04 from one codebase with one designer, which no single GTK version
does.

That is not the chosen plan. It is recorded in case the split between a frozen
gtkmm 2.4 tier and a GTK 4 tier becomes expensive, since a Qt 5.15 port would
collapse Tiers 1 and 2 into one.

---

## Status of claims in this document

Verified in this repository:

- `master` requires pkg-config `gtkmm-2.4` and `libglademm-2.4`, and sets
  `-std=c++11`.
- GTK 3 was never ported: `385a188` (2014-10-22) changed the pkg-config module
  to `gtkmm-3.0` and `246c7a7` (2016-02-29) changed it back; no commit in the
  whole history touches `src/` for `gtkmm-3.0`, and `libglademm-2.4` was never
  changed.
- `external/` contains `gtkmm-win32-devel-2.22.0-2.exe` (and 2.16.0), and
  `run/` ships `libgtk-win32-2.0-0.dll` with `gtkmm-vc90-2_4.dll` and
  `glademm-vc90-2_4.dll` — so the Windows runtime is the gtkmm 2.22 generation.
- `resources/legacy-gtk2/burza.glade` is libglade format
  (`<glade-interface>`, `interface-requires gtk+ 2.16`).
- The gtkmm 2-only API counts in `master`'s `src/bdialogs.cpp`
  (9 × `Gnome::Glade::Xml`, 8 × `get_widget_derived`, 4 × `Gdk::Color`,
  2 × `Gtk::Stock`).
- `std::format` appears in exactly the four places listed, and is the only
  C++20 feature present (`<=>` matches only a comment in `convfncs.cpp`).
- The GTK symbol counts and the three core → GUI call sites.
- `burza_kdev_4/CMakeLists.txt` on `Cambalache` configures on CMake 4.2.3 and
  exposes `BURZA_CXX_STANDARD` (11/14/17/20).
- Only `master` and `Cambalache` exist as branches; there is no `Gtk2` or
  `Gtk3`.

Not verified — check before relying on:

- The Tier 2 build has not been compiled end to end: gtkmm, libpqxx and ICU are
  not installed on the development machine, so only the CMake configure step
  was tested.
- Windows toolchain claims (v141_xp, v142 Win7 targeting) are from
  documentation, not from a build on this project.
- Ubuntu 18.04 package versions (`libgtkmm-2.4-dev` 2.24.5, `libgtk2.0-dev`
  2.24.32, Glade 3.22) are from the distribution's archive, not from a live
  18.04 machine.
- Glade's GTK 2 support ending at 3.8, and Cambalache having no GTK 2 target,
  are from those projects' documentation.
- The exact GTK+ 2 version each tier historically ran is inferred from the
  bundle names and DLLs listed above, not read out of a binary: the GTK 2
  module ABI directory is `2.10.0` for every release from 2.10 to 2.24, so it
  does not identify the version. "GTK+ 2.2x" is deliberate where the precise
  number is not established.
