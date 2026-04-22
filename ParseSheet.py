from __future__ import annotations

import csv
import json
import shutil
import subprocess
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Iterable, Sequence


SHEET_ID = "1Kc9aME3xlUC_vV5dFRe457OchqUOrwuiX_pQykjCF68"
SHEET_FOLDER = Path("SheetData")
DATA_FOLDER = Path("Data")

PARTS_SHEET = SHEET_FOLDER / "parts.csv"
PARTS_SHEET_GID = "503295784"

PARTS_V2_SHEET = SHEET_FOLDER / "parts2.csv"
PARTS_V2_SHEET_GID = "319672878"

CORES_SHEET = SHEET_FOLDER / "cores.csv"
CORES_SHEET_GID = "911413911"

OUTPUT_FILE = DATA_FOLDER / "FullData.json"
CATEGORIES_FILE = DATA_FOLDER / "Categories.json"
PENALTIES_FILE = DATA_FOLDER / "Penalties.json"

VALID_PART_CATEGORIES = ["AR", "Sniper", "SMG", "LMG", "Shotgun", "BR", "Weird", "Sidearm"]
VALID_PART_TYPES = ["Barrels", "Magazines", "Grips", "Stocks"]
VALID_PRICE_TYPES = [
    "Coin",
    "WC",
    "Follow",
    "Robux",
    "Free",
    "Spin",
    "Limited",
    "Missions",
    "Verify discord",
    "Season Pass 1",
    "Unknown",
]

PART_PROPERTY_NAMES = [
    "Magazine_Size",
    "Reload_Time",
    "Damage",
    "Detection_Radius",
    "Equip_Time",
    "Fire_Rate",
    "Health",
    "Magazine_Cap",
    "Movement_Speed",
    "Pellets",
    "Range",
    "Recoil",
    "Reload_Speed",
    "Spread",
]

CORE_PROPERTY_NAMES = [
    "Damage",
    "Dropoff_Studs",
    "Fire_Rate",
    "Hipfire_Spread",
    "ADS_Spread",
    "Time_To_Aim",
    "Detection_Radius",
    "Burst",
    "Movement_Speed_Modifier",
    "Suppression",
    "Health",
    "Equip_Time",
    "Recoil_Hip_Horizontal",
    "Recoil_Hip_Vertical",
    "Recoil_Aim_Horizontal",
    "Recoil_Aim_Vertical",
]

CURRENT_CATEGORIES = {
    "Primary": {
        "AR": 0,
        "Sniper": 1,
        "SMG": 2,
        "Shotgun": 3,
        "LMG": 4,
        "Weird": 5,
        "BR": 6,
    },
    "Secondary": {
        "Sidearm": 7,
    },
}

CURRENT_PENALTIES = [
    [1.00, 0.70, 0.75, 0.70, 0.75, 1.00, 0.80, 0.65],
    [0.70, 1.00, 0.60, 0.60, 0.80, 1.00, 0.85, 0.50],
    [0.80, 0.60, 1.00, 0.65, 0.65, 1.00, 0.70, 0.70],
    [0.70, 0.50, 0.65, 1.00, 0.75, 1.00, 0.60, 0.65],
    [0.75, 0.80, 0.65, 0.75, 1.00, 1.00, 0.85, 0.50],
    [1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00],
    [0.80, 0.85, 0.70, 0.60, 0.85, 1.00, 1.00, 0.65],
    [0.65, 0.50, 0.75, 0.65, 0.50, 1.00, 0.65, 1.00],
]


class ParseError(ValueError):
    """Raised when sheet data cannot be parsed safely."""


@dataclass(frozen=True)
class GoogleSheetExport:
    gid: str
    output_path: Path

    def export_url(self, sheet_id: str) -> str:
        return (
            f"https://docs.google.com/spreadsheets/d/{sheet_id}/export"
            f"?format=csv&id={sheet_id}&gid={self.gid}"
        )


class SheetDownloader:
    def __init__(self, sheet_id: str, sheet_folder: Path) -> None:
        self.sheet_id = sheet_id
        self.sheet_folder = sheet_folder

    def clear_sheet_folder(self) -> None:
        self.sheet_folder.mkdir(parents=True, exist_ok=True)
        for child in self.sheet_folder.iterdir():
            if child.is_file():
                child.unlink()
            elif child.is_dir():
                shutil.rmtree(child)

    def download(self, exports: Sequence[GoogleSheetExport]) -> None:
        self.clear_sheet_folder()
        for export in exports:
            self._download_file(export.export_url(self.sheet_id), export.output_path)

    @staticmethod
    def _download_file(url: str, output_path: Path) -> None:
        output_path.parent.mkdir(parents=True, exist_ok=True)
        result = subprocess.run(
            ["wget", "-O", str(output_path), url],
            capture_output=True,
            text=True,
            check=False,
        )
        if result.returncode != 0:
            raise RuntimeError(
                f"Failed to download sheet to {output_path}. "
                f"stderr={result.stderr.strip()}"
            )


def read_csv_rows(path: Path, *, skip_header_rows: int = 2, trim_first_column: bool = True) -> list[list[str]]:
    with path.open("r", newline="", encoding="utf-8") as file:
        rows = list(csv.reader(file))

    data_rows = rows[skip_header_rows:]
    if trim_first_column:
        return [row[1:] for row in data_rows]
    return data_rows


def normalize_numeric_value(raw_value: str, *, expect_range: bool = False) -> int | float | list[int | float] | None:
    value = raw_value.strip()
    if value in {"", "🎲"}:
        return None

    cleaned = (
        value.replace("°", "")
        .replace("s", "")
        .replace("rpm", "")
        .replace("%", "")
        .replace(",", "")
        .replace(">", "-")
        .strip()
    )

    if expect_range:
        if " - " not in cleaned:
            raise ParseError(f"Expected numeric range but got: {raw_value!r}")

        values = cleaned.split(" - ")
        if len(values) != 2:
            raise ParseError(f"Expected two values in range but got: {raw_value!r}")

        parsed = [_parse_single_or_multiplier(item) for item in values]
        return [_coerce_number(number) for number in parsed]

    return _coerce_number(_parse_single_or_multiplier(cleaned))


def _parse_single_or_multiplier(value: str) -> float:
    if "x" in value:
        left, right = value.split("x", maxsplit=1)
        return float(left) * float(right)
    return float(value)


def _coerce_number(value: float) -> int | float:
    return int(value) if int(value) == value else value


def detect_price_type(price: str, row_context: Sequence[str] | None = None) -> str:
    normalized = price.strip()

    if normalized == "":
        return "Coin"

    if normalized in VALID_PRICE_TYPES:
        return normalized

    capitalized = normalized.capitalize()
    if capitalized in VALID_PRICE_TYPES:
        return capitalized

    aliases = {
        "Weird Boxes": "WC",
        "Exclusive Weird Boxes": "Robux",
    }
    if normalized in aliases:
        return aliases[normalized]

    numeric_candidate = normalized.replace(",", "")
    if "WC" in numeric_candidate:
        return "WC"

    try:
        int(numeric_candidate)
        return "Coin"
    except ValueError:
        if capitalized not in VALID_PRICE_TYPES:
            print(f"WARNING: Invalid price type detected for row {row_context}")
            return "Unknown"
        return capitalized


def find_same_name(items: Iterable[dict[str, Any]], name: str) -> dict[str, Any] | None:
    for item in items:
        if item.get("Name") == name:
            return item
    return None


def deep_compare(expected: dict[str, list[dict[str, Any]]], actual: dict[str, list[dict[str, Any]]]) -> None:
    for part_type, expected_items in expected.items():
        print(f"\n------------------------- {part_type} -----------------------------\n")
        for expected_item in expected_items:
            if expected_item.get("Name") == "Stat Randomizer":
                continue

            actual_item = find_same_name(actual.get(part_type, []), expected_item["Name"])
            if actual_item is None:
                print(f"Part {expected_item['Name']} not found in {part_type}")
                continue

            if expected_item != actual_item:
                print(expected_item, "\n", actual_item)
                print()


class PartsParser:
    def __init__(self) -> None:
        self._seen_parts: dict[tuple[str, str], set[str]] = {
            (category, part_type): set()
            for category in VALID_PART_CATEGORIES
            for part_type in VALID_PART_TYPES
        }

    def parse_rows(self, rows: Sequence[Sequence[str]]) -> dict[str, list[dict[str, Any]]]:
        output: dict[str, list[dict[str, Any]]] = {
            "Barrels": [],
            "Magazines": [],
            "Grips": [],
            "Stocks": [],
        }

        current_category = "AR"
        current_type = ""

        for row in rows:
            if not row:
                continue
            if len(row) != 16:
                raise ParseError(f"Invalid parts row length: expected 16, got {len(row)}. row={row}")

            name = row[1].strip()
            divider = self._parse_divider(name)
            if divider is not None:
                current_category, current_type = divider
                continue

            if current_type == "":
                raise ParseError(f"Encountered part before any part type header. row={row}")

            seen_key = (current_category, current_type)
            if name in self._seen_parts[seen_key]:
                raise ParseError(
                    f"Duplicate part name {name!r} in category {current_category!r} and part type {current_type!r}"
                )
            self._seen_parts[seen_key].add(name)

            part: dict[str, Any] = {
                "Price_Type": detect_price_type(row[0], row),
                "Name": name,
                "Category": current_category,
            }

            for index in range(2, 16):
                cell = row[index].strip()
                if not cell:
                    continue

                token_value = _extract_leading_token(cell)
                property_name = PART_PROPERTY_NAMES[index - 2]
                part[property_name] = normalize_numeric_value(token_value)

            output[current_type].append(part)

        return output

    @staticmethod
    def _parse_divider(name: str) -> tuple[str, str] | None:
        parts = name.split(" ")
        if len(parts) != 2:
            return None

        if parts[0] == "Notable":
            raise StopIteration  # sentinel used by caller in file-based parse if desired

        category, part_type = parts
        if category in VALID_PART_CATEGORIES and part_type in VALID_PART_TYPES:
            return category, part_type
        return None

    def parse_file(self, path: Path) -> dict[str, list[dict[str, Any]]]:
        rows = read_csv_rows(path)
        truncated_rows: list[list[str]] = []

        for row in rows:
            if len(row) >= 2 and row[1].strip().startswith("Notable "):
                break
            truncated_rows.append(row)

        return self.parse_rows(truncated_rows)


class CoresParser:
    def parse_rows(self, rows: Sequence[Sequence[str]]) -> list[dict[str, Any]]:
        output: list[dict[str, Any]] = []
        current_category = "AR"

        for row in rows:
            if not row:
                continue
            if len(row) != 18:
                raise ParseError(f"Invalid cores row length: expected 18, got {len(row)}. row={row}")

            name = row[1].strip()
            if self._is_category_divider(name):
                current_category = name[:-6]
                continue

            core: dict[str, Any] = {
                "Price_Type": detect_price_type(row[0], row),
                "Name": name,
                "Category": current_category,
            }

            for index in range(2, 18):
                cell = row[index]

                if index == 2:
                    pellets = self._extract_pellets(cell)
                    if pellets is not None:
                        core["Pellets"] = pellets

                value = normalize_numeric_value(cell, expect_range=(index <= 3 or index >= 14))
                if value is not None:
                    core[CORE_PROPERTY_NAMES[index - 2]] = value

            output.append(core)

        return output

    @staticmethod
    def _is_category_divider(name: str) -> bool:
        return name.endswith(" Cores") and name[:-6] in VALID_PART_CATEGORIES

    @staticmethod
    def _extract_pellets(cell: str) -> int | None:
        first_segment = cell.split(" > ")[0]
        pieces = first_segment.split("x")
        if len(pieces) == 2:
            return int(pieces[1])
        return None

    def parse_file(self, path: Path) -> list[dict[str, Any]]:
        rows = read_csv_rows(path)
        return self.parse_rows(rows)


def _extract_leading_token(cell: str) -> str:
    if " " not in cell:
        raise ParseError(f"Invalid property cell format: {cell!r}")
    return cell.split(" ", maxsplit=1)[0]


def save_json(data: Any, output_path: Path, *, indent: int = 2) -> None:
    output_path.parent.mkdir(parents=True, exist_ok=True)
    with output_path.open("w", encoding="utf-8") as file:
        json.dump(data, file, indent=indent)


def build_full_data(parts_file: Path = PARTS_V2_SHEET, cores_file: Path = CORES_SHEET) -> dict[str, Any]:
    parts_data = PartsParser().parse_file(parts_file)
    cores_data = CoresParser().parse_file(cores_file)

    combined_data = {
        "Barrels": parts_data["Barrels"],
        "Magazines": parts_data["Magazines"],
        "Grips": parts_data["Grips"],
        "Stocks": parts_data["Stocks"],
        "Cores": cores_data,
    }

    return {
        "Data": combined_data,
        "Penalties": CURRENT_PENALTIES,
        "Categories": CURRENT_CATEGORIES,
    }


def write_penalty_files() -> None:
    print("Running Penalties")
    save_json(CURRENT_CATEGORIES, CATEGORIES_FILE)
    save_json(CURRENT_PENALTIES, PENALTIES_FILE, indent=None)


def compare_with_existing(reference_path: Path = OUTPUT_FILE, generated_path: Path = OUTPUT_FILE) -> None:
    with reference_path.open("r", encoding="utf-8") as file:
        reference_data = json.load(file)

    with generated_path.open("r", encoding="utf-8") as file:
        generated_data = json.load(file)

    deep_compare(reference_data, generated_data)


def download_sheets() -> None:
    downloader = SheetDownloader(SHEET_ID, SHEET_FOLDER)
    downloader.download(
        [
            GoogleSheetExport(gid=CORES_SHEET_GID, output_path=CORES_SHEET),
            GoogleSheetExport(gid=PARTS_V2_SHEET_GID, output_path=PARTS_V2_SHEET),
        ]
    )


def main() -> None:
    download_sheets()
    full_data = build_full_data()
    save_json(full_data, OUTPUT_FILE)


if __name__ == "__main__":
    main()
