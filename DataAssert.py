# <3 from Gemini 3.8

"""
DataAssert.py - Dataset Assertion and Validation Suite for WeirdGunGameCalc.

Validates Data/FullData.json according to wggcalc specifications:
1. Gun name uniqueness per part category and capitalization consistency across the dataset.
2. Key validation against keys read by wggcalc (catching typos, wrong keys, and casing issues).
3. Magazine-specific required properties (Magazine_Size and Reload_Time).
4. Non-negative number constraints (>= 0) on specified core and magazine statistics
   (0 is permitted for Stat Randomizer to model purely random stats in game).
5. Structural integrity ensuring the JSON is not malformed.
Includes exact line numbers for assertion failures in both the JSON dataset and DataAssert.py.
"""

import argparse
import bisect
import inspect
import json
import json.decoder
import json.scanner
import sys
from pathlib import Path
from typing import Any, Dict, List, Optional, Set, Tuple


class DataAssertionError(AssertionError):
    """
    Raised when an assertion on the dataset fails.
    Carries both the JSON line number (where the issue is located in the dataset file)
    and the Python line number in DataAssert.py (where the assertion failed).
    """

    def __init__(
        self,
        message: str,
        json_line: Optional[int] = None,
        py_line: Optional[int] = None,
    ):
        self.raw_message = message
        self.json_line = json_line
        self.py_line = py_line

        loc_tags: List[str] = []
        if json_line is not None:
            loc_tags.append(f"Line {json_line}")
        if py_line is not None:
            loc_tags.append(f"DataAssert.py:{py_line}")

        prefix = f"[{' | '.join(loc_tags)}] " if loc_tags else ""
        super().__init__(f"{prefix}{message}")


# ----------------------------------------------------------------------
# Line-Tracking JSON Decoder
# ----------------------------------------------------------------------

class LineNumberDecoder(json.JSONDecoder):
    """
    Custom JSON decoder that annotates every parsed JSON dictionary with:
    - '__line__': The 1-based line number where the object starts in the JSON file.
    - '__key_lines__': A dict mapping each key to the line number where it appeared.
    These metadata fields allow assertions to pinpoint the exact line in the dataset file.
    """

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.parse_object = self._custom_parse_object
        self.scan_once = json.scanner.py_make_scanner(self)
        self.line_offsets: List[int] = []

    def decode(self, s: str, *args, **kwargs) -> Any:
        self.line_offsets = [0]
        for idx, char in enumerate(s):
            if char == "\n":
                self.line_offsets.append(idx + 1)
        return super().decode(s, *args, **kwargs)

    def get_line(self, char_idx: int) -> int:
        """Convert a 0-based character offset to a 1-based line number."""
        return bisect.bisect_right(self.line_offsets, char_idx)

    def _custom_parse_object(
        self,
        s_and_end: Tuple[str, int],
        strict: bool,
        scan_once: Any,
        object_hook: Any,
        object_pairs_hook: Any,
        memo: Optional[Dict[str, str]] = None,
        _w: Any = json.decoder.WHITESPACE.match,
        _ws: str = json.decoder.WHITESPACE_STR,
    ) -> Tuple[Dict[str, Any], int]:
        s, end = s_and_end
        obj_start = end - 1
        pairs: List[Tuple[str, Any]] = []
        pairs_append = pairs.append
        if memo is None:
            memo = {}
        memo_get = memo.setdefault

        nextchar = s[end:end + 1]
        if nextchar != '"':
            if nextchar in _ws:
                end = _w(s, end).end()
                nextchar = s[end:end + 1]
            if nextchar == "}":
                res = {"__line__": self.get_line(obj_start), "__key_lines__": {}}
                return res, end + 1
            elif nextchar != '"':
                raise json.decoder.JSONDecodeError(
                    "Expecting property name enclosed in double quotes", s, end
                )
        end += 1

        key_lines: Dict[str, int] = {}
        while True:
            key_start = end - 1
            key, end = json.decoder.scanstring(s, end, strict)
            key_lines[key] = self.get_line(key_start)
            key = memo_get(key, key)

            if s[end:end + 1] != ":":
                end = _w(s, end).end()
                if s[end:end + 1] != ":":
                    raise json.decoder.JSONDecodeError("Expecting ':' delimiter", s, end)
            end += 1

            try:
                if s[end] in _ws:
                    end += 1
                    if s[end] in _ws:
                        end = _w(s, end + 1).end()
            except IndexError:
                pass

            try:
                val, end = scan_once(s, end)
            except StopIteration as err:
                raise json.decoder.JSONDecodeError("Expecting value", s, err.value) from None

            pairs_append((key, val))

            try:
                nextchar = s[end]
                if nextchar in _ws:
                    end = _w(s, end + 1).end()
                    nextchar = s[end]
            except IndexError:
                nextchar = ""
            end += 1

            if nextchar == "}":
                break
            elif nextchar != ",":
                raise json.decoder.JSONDecodeError("Expecting ',' delimiter", s, end - 1)

            comma_idx = end - 1
            end = _w(s, end).end()
            nextchar = s[end:end + 1]
            end += 1
            if nextchar != '"':
                if nextchar == "}":
                    raise json.decoder.JSONDecodeError(
                        "Illegal trailing comma before end of object", s, comma_idx
                    )
                raise json.decoder.JSONDecodeError(
                    "Expecting property name enclosed in double quotes", s, end - 1
                )

        result = dict(pairs)
        result["__line__"] = self.get_line(obj_start)
        result["__key_lines__"] = key_lines
        return result, end


# Helper functions to extract line numbers and raise formatted errors
def _get_json_line(item: Any, key: Optional[str] = None) -> Optional[int]:
    """Retrieve the JSON line number for an item or specific key if available."""
    if isinstance(item, dict):
        if key and "__key_lines__" in item and key in item["__key_lines__"]:
            return item["__key_lines__"][key]
        if "__line__" in item:
            return item["__line__"]
    return None


def _get_caller_py_line(depth: int = 2) -> Optional[int]:
    """Retrieve the line number in DataAssert.py where the assertion failed."""
    frame = inspect.currentframe()
    for _ in range(depth):
        if frame and frame.f_back:
            frame = frame.f_back
        else:
            break
    return frame.f_lineno if frame else None


def _fail(
    msg: str,
    item: Any = None,
    key: Optional[str] = None,
    json_line: Optional[int] = None,
    py_line: Optional[int] = None,
) -> None:
    """Raise a DataAssertionError annotated with JSON and Python line numbers."""
    if json_line is None:
        json_line = _get_json_line(item, key)
    if py_line is None:
        py_line = _get_caller_py_line(depth=2)
    raise DataAssertionError(msg, json_line=json_line, py_line=py_line)


def _format_error_entry(
    msg: str,
    item: Any = None,
    key: Optional[str] = None,
    json_line: Optional[int] = None,
    py_line: Optional[int] = None,
) -> str:
    """Format an error message with location tags for collective reporting."""
    if json_line is None:
        json_line = _get_json_line(item, key)
    if py_line is None:
        py_line = _get_caller_py_line(depth=2)

    loc_tags: List[str] = []
    if json_line is not None:
        loc_tags.append(f"Line {json_line}")
    if py_line is not None:
        loc_tags.append(f"DataAssert.py:{py_line}")

    prefix = f"[{' | '.join(loc_tags)}] " if loc_tags else ""
    return f"{prefix}{msg}"


# ----------------------------------------------------------------------
# Constants based on wggcalc.hpp and central program requirements
# ----------------------------------------------------------------------

PART_CATEGORIES = ("Barrels", "Magazines", "Grips", "Stocks", "Cores")

# Base properties read by Part constructor in wggcalc.hpp (lines 811-841)
VALID_PART_KEYS: Set[str] = {
    "Category",
    "Name",
    "Price_Type",
    "Damage",
    "Fire_Rate",
    "Spread",
    "Recoil",
    "Equip_Time",
    "Pellets",
    "Movement_Speed",
    "Reload_Speed",
    "Magazine_Cap",
    "Health",
    "Detection_Radius",
    "Range",
    "Spin_Up",
    "Blast_Radius",
}

# Magazine inherits from Part and reads Magazine_Size & Reload_Time (wggcalc.hpp lines 934-939)
VALID_MAGAZINE_KEYS: Set[str] = VALID_PART_KEYS | {
    "Magazine_Size",
    "Reload_Time",
}

# Core properties read by Core constructor in wggcalc.hpp (lines 661-746)
VALID_CORE_KEYS: Set[str] = {
    "Category",
    "Name",
    "Firing_Mode",
    "Price_Type",
    "Damage",
    "Dropoff_Studs",
    "Falloff_Factor",
    "Fire_Rate",
    "Hipfire_Spread",
    "ADS_Spread",
    "Time_To_Aim",
    "Movement_Speed_Modifier",
    "Pellets",
    "Burst",
    "Detection_Radius",
    "Health",
    "Equip_Time",
    "Spin_Up",
    "Blast_Radius",
    "Recoil_Hip_Horizontal",
    "Recoil_Hip_Vertical",
    "Recoil_Aim_Horizontal",
    "Recoil_Aim_Vertical",
    # Suppression is exported in the dataset for cores and used by web types
    "Suppression",
}

# Core properties that must be non-negative (>= 0) when present
# (0 is allowed, e.g. Stat Randomizer fire rate)
NON_NEGATIVE_CORE_PROPERTIES: Tuple[str, ...] = (
    "Pellets",
    "Burst",
    "Detection_Radius",
    "Blast_Radius",
    "Time_To_Aim",
    "Equip_Time",
    "Hipfire_Spread",
    "ADS_Spread",
    "Fire_Rate",
)

# Magazine properties that must be non-negative (>= 0)
NON_NEGATIVE_MAGAZINE_PROPERTIES: Tuple[str, ...] = (
    "Magazine_Size",
    "Reload_Time",
)

# Backward-compatibility aliases
POSITIVE_CORE_PROPERTIES = NON_NEGATIVE_CORE_PROPERTIES
POSITIVE_MAGAZINE_PROPERTIES = NON_NEGATIVE_MAGAZINE_PROPERTIES


# ----------------------------------------------------------------------
# 5. Assert that JSON is not malformed
# ----------------------------------------------------------------------

def assert_not_malformed(data: Any) -> None:
    """
    Assert that the dataset structure is well-formed:
    - Root is a dictionary containing 'Categories', 'Penalties', and 'Data'.
    - 'Categories' contains 'Primary' and 'Secondary' dicts with string names and int IDs.
    - 'Penalties' is a square 2D matrix matching total category count.
    - 'Data' contains all 5 part categories, each as a list of valid dictionaries.
    - Each item has valid 'Name' and 'Category' fields, and Cores have 'Firing_Mode'.
    """
    if not isinstance(data, dict):
        _fail(f"Root JSON must be a dictionary/object, got {type(data).__name__}")

    for top_key in ("Categories", "Penalties", "Data"):
        if top_key not in data:
            _fail(f"Missing required top-level key: '{top_key}'", item=data)

    # Validate Categories
    categories = data["Categories"]
    if not isinstance(categories, dict):
        _fail(f"'Categories' must be a dictionary, got {type(categories).__name__}", item=data, key="Categories")
    if "Primary" not in categories or "Secondary" not in categories:
        _fail("'Categories' must contain both 'Primary' and 'Secondary'", item=categories)

    primary_cats = categories["Primary"]
    secondary_cats = categories["Secondary"]
    if not isinstance(primary_cats, dict) or not isinstance(secondary_cats, dict):
        _fail("'Primary' and 'Secondary' in Categories must be dictionaries", item=categories)

    all_categories: Dict[str, int] = {}
    combined_cats = {
        k: v for k, v in {**primary_cats, **secondary_cats}.items()
        if not k.startswith("__")
    }
    for cat_name, cat_id in combined_cats.items():
        if not isinstance(cat_name, str) or not cat_name.strip():
            _fail(f"Category name must be a non-empty string, got {cat_name!r}", item=categories)
        if not isinstance(cat_id, int) or isinstance(cat_id, bool):
            _fail(f"Category ID for '{cat_name}' must be an integer, got {cat_id!r}", item=categories)
        all_categories[cat_name] = cat_id

    total_categories = len(all_categories)
    if total_categories == 0:
        _fail("Categories must not be empty", item=categories)

    expected_ids = set(range(total_categories))
    actual_ids = set(all_categories.values())
    if actual_ids != expected_ids:
        _fail(f"Category IDs must be continuous 0 to {total_categories - 1}, got {actual_ids}", item=categories)

    # Validate Penalties matrix
    penalties = data["Penalties"]
    if not isinstance(penalties, list):
        _fail(f"'Penalties' must be a list/2D-array, got {type(penalties).__name__}", item=data, key="Penalties")
    if len(penalties) != total_categories:
        _fail(
            f"'Penalties' row count ({len(penalties)}) must match category count ({total_categories})",
            item=data,
            key="Penalties",
        )
    for row_idx, row in enumerate(penalties):
        if not isinstance(row, list):
            _fail(f"Penalties row {row_idx} must be a list", item=data, key="Penalties")
        if len(row) != total_categories:
            _fail(
                f"Penalties row {row_idx} length ({len(row)}) must match category count ({total_categories})",
                item=data,
                key="Penalties",
            )
        for col_idx, val in enumerate(row):
            if not isinstance(val, (int, float)) or isinstance(val, bool):
                _fail(
                    f"Penalties[{row_idx}][{col_idx}] must be a float/int, got {val!r}",
                    item=data,
                    key="Penalties",
                )

    # Validate Data dictionary
    data_section = data["Data"]
    if not isinstance(data_section, dict):
        _fail(f"'Data' must be a dictionary, got {type(data_section).__name__}", item=data, key="Data")

    for part_type in PART_CATEGORIES:
        if part_type not in data_section:
            _fail(f"'Data' is missing required category list: '{part_type}'", item=data_section)
        part_list = data_section[part_type]
        if not isinstance(part_list, list):
            _fail(f"'Data.{part_type}' must be a list, got {type(part_list).__name__}", item=data_section, key=part_type)

        for idx, item in enumerate(part_list):
            if not isinstance(item, dict):
                _fail(f"Item #{idx} in '{part_type}' must be a dictionary", item=data_section, key=part_type)
            if "Name" not in item:
                _fail(f"Item #{idx} in '{part_type}' is missing required key 'Name'", item=item)
            if not isinstance(item["Name"], str) or not item["Name"].strip():
                _fail(f"Item #{idx} in '{part_type}' has invalid or empty Name", item=item, key="Name")
            if "Category" not in item:
                _fail(f"Item '{item['Name']}' in '{part_type}' is missing required key 'Category'", item=item)
            if item["Category"] not in all_categories:
                _fail(
                    f"Item '{item['Name']}' in '{part_type}' has unknown Category '{item['Category']}'. "
                    f"Valid categories: {list(all_categories.keys())}",
                    item=item,
                    key="Category",
                )
            if part_type == "Cores":
                if "Firing_Mode" not in item:
                    _fail(f"Core '{item['Name']}' is missing required key 'Firing_Mode'", item=item)
                if not isinstance(item["Firing_Mode"], str) or not item["Firing_Mode"].strip():
                    _fail(f"Core '{item['Name']}' has invalid or empty 'Firing_Mode'", item=item, key="Firing_Mode")

                # Validate paired properties if present on cores
                for pair_prop in (
                    "Dropoff_Studs",
                    "Recoil_Hip_Horizontal",
                    "Recoil_Hip_Vertical",
                    "Recoil_Aim_Horizontal",
                    "Recoil_Aim_Vertical",
                ):
                    if pair_prop in item:
                        val = item[pair_prop]
                        if not isinstance(val, list) or len(val) != 2:
                            _fail(
                                f"Core '{item['Name']}' property '{pair_prop}' must be a pair of 2 numbers, got {val!r}",
                                item=item,
                                key=pair_prop,
                            )
                        if not all(isinstance(v, (int, float)) and not isinstance(v, bool) for v in val):
                            _fail(
                                f"Core '{item['Name']}' property '{pair_prop}' values must be numbers, got {val!r}",
                                item=item,
                                key=pair_prop,
                            )


# ----------------------------------------------------------------------
# 1. Assert gun name uniqueness per category and consistent capitalization
# ----------------------------------------------------------------------

def assert_unique_names_and_capitalization(data: Dict[str, Any]) -> None:
    """
    1. For each gun, every instance of that gun has to only be part of a category once
       (e.g., no two barrels named the same, no two magazines named the same, etc.).
    2. Ensure each instance of the same name has the same capitalization across the dataset
       (verified by comparing the regular string with a canonical mapping keyed by lowercased version).
    3. Ensure that all parts sharing the same gun name belong to the same weapon category.
    """
    data_section = data["Data"]
    canonical_names: Dict[str, str] = {}  # lowercased_name -> original_name
    gun_categories: Dict[str, str] = {}   # lowercased_name -> weapon_category

    for part_type in PART_CATEGORIES:
        seen_in_this_category: Set[str] = set()

        for item in data_section[part_type]:
            name = item["Name"]
            lower_name = name.lower()
            category = item.get("Category", "")

            # Check 1: No two items in the same part category share the same name
            if lower_name in seen_in_this_category:
                _fail(
                    f"Duplicate gun name found in category '{part_type}': '{name}' "
                    f"(a gun can only be part of a category once)",
                    item=item,
                    key="Name",
                )
            seen_in_this_category.add(lower_name)

            # Check 2: Consistent capitalization across the entire dataset
            if lower_name in canonical_names:
                expected_name = canonical_names[lower_name]
                if name != expected_name:
                    _fail(
                        f"Inconsistent capitalization for gun name '{name}' in '{part_type}'. "
                        f"Expected '{expected_name}' as previously defined.",
                        item=item,
                        key="Name",
                    )
            else:
                canonical_names[lower_name] = name

            # Check 3: All parts of the same gun share the same weapon Category
            if lower_name in gun_categories:
                expected_category = gun_categories[lower_name]
                if category != expected_category:
                    _fail(
                        f"Gun '{name}' belongs to category '{expected_category}', but item in "
                        f"'{part_type}' specifies category '{category}'",
                        item=item,
                        key="Category",
                    )
            else:
                gun_categories[lower_name] = category


# ----------------------------------------------------------------------
# 2. Assert key validity and correct capitalization
# ----------------------------------------------------------------------

def assert_valid_keys(data: Dict[str, Any], allow_suppression: bool = True) -> None:
    """
    Ensure that all keys found inside the json file are valid keys actually read by wggcalc
    (or known dataset properties). None may be misspelled or have incorrect capitalization.
    """
    data_section = data["Data"]

    core_valid = set(VALID_CORE_KEYS)
    if not allow_suppression:
        core_valid.discard("Suppression")

    part_keys_lower = {k.lower(): k for k in VALID_PART_KEYS}
    mag_keys_lower = {k.lower(): k for k in VALID_MAGAZINE_KEYS}
    core_keys_lower = {k.lower(): k for k in core_valid}

    for part_type in PART_CATEGORIES:
        if part_type == "Magazines":
            valid_keys = VALID_MAGAZINE_KEYS
            lower_lookup = mag_keys_lower
        elif part_type == "Cores":
            valid_keys = core_valid
            lower_lookup = core_keys_lower
        else:
            valid_keys = VALID_PART_KEYS
            lower_lookup = part_keys_lower

        for item in data_section[part_type]:
            name = item.get("Name", "<unknown>")
            for key in item.keys():
                # Ignore internal metadata keys attached by LineNumberDecoder
                if key.startswith("__"):
                    continue

                if key in valid_keys:
                    continue

                # Key is not in valid_keys: diagnose why
                key_lower = key.lower()

                # Specific diagnosis: Movement_Speed on Cores
                if part_type == "Cores" and key == "Movement_Speed":
                    _fail(
                        f"Invalid key 'Movement_Speed' in Core '{name}'. "
                        f"wggcalc expects 'Movement_Speed_Modifier' on Cores.",
                        item=item,
                        key=key,
                    )

                # Check if it is a capitalization mismatch
                if key_lower in lower_lookup:
                    canonical_key = lower_lookup[key_lower]
                    _fail(
                        f"Incorrect capitalization for key '{key}' in {part_type} '{name}'. "
                        f"Expected '{canonical_key}'.",
                        item=item,
                        key=key,
                    )

                # Check if it belongs to another part type
                if part_type != "Magazines" and key in ("Magazine_Size", "Reload_Time"):
                    _fail(
                        f"Key '{key}' in {part_type} '{name}' is only valid on Magazines.",
                        item=item,
                        key=key,
                    )
                if part_type != "Cores" and key in (
                    "Hipfire_Spread",
                    "ADS_Spread",
                    "Time_To_Aim",
                    "Burst",
                    "Movement_Speed_Modifier",
                ):
                    _fail(
                        f"Key '{key}' in {part_type} '{name}' is only valid on Cores.",
                        item=item,
                        key=key,
                    )

                _fail(
                    f"Unknown/unrecognized key '{key}' in {part_type} '{name}'. ",
                    item=item,
                    key=key,
                )


# ----------------------------------------------------------------------
# 3. Assert magazine-specific properties
# ----------------------------------------------------------------------

def assert_magazine_properties(data: Dict[str, Any]) -> None:
    """
    Assert that all magazines contain both 'Magazine_Size' and 'Reload_Time'.
    """
    magazines = data["Data"]["Magazines"]
    for mag in magazines:
        name = mag.get("Name", "<unknown>")

        if "Magazine_Size" not in mag:
            _fail(f"Magazine '{name}' is missing required property 'Magazine_Size'", item=mag)
        if mag["Magazine_Size"] is None:
            _fail(f"Magazine '{name}' has null value for 'Magazine_Size'", item=mag, key="Magazine_Size")

        if "Reload_Time" not in mag:
            _fail(f"Magazine '{name}' is missing required property 'Reload_Time'", item=mag)
        if mag["Reload_Time"] is None:
            _fail(f"Magazine '{name}' has null value for 'Reload_Time'", item=mag, key="Reload_Time")


# ----------------------------------------------------------------------
# 4. Assert non-negative numbers (>= 0)
# ----------------------------------------------------------------------

def assert_non_negative_numbers(data: Dict[str, Any]) -> None:
    """
    Assert non-negative numbers (>= 0) for:
    - Magazine_Size (Magazines)
    - Reload_Time (Magazines)
    - Pellets (Only on the core, when present)
    - Burst (Only on the core, when present)
    - Detection_Radius (Only on the core, when present)
    - Blast_Radius (Only on the core, when present)
    - Time_To_Aim (Only on the core, when present)
    - Equip_Time (Only on the core, when present)
    - Hipfire_Spread (Only on the core, when present)
    - ADS_Spread (Only on the core, when present)
    - Fire_Rate (Only on the core, when present)

    Note: Non-negative (>= 0) is required instead of strictly positive (> 0)
    because special weapons such as 'Stat Randomizer' possess a baseline of 0
    to model purely randomized runtime stats in game.
    """
    data_section = data["Data"]

    # 1. Magazine checks
    for mag in data_section["Magazines"]:
        name = mag.get("Name", "<unknown>")
        for prop in NON_NEGATIVE_MAGAZINE_PROPERTIES:
            if prop in mag:
                val = mag[prop]
                if not isinstance(val, (int, float)) or isinstance(val, bool):
                    _fail(
                        f"Magazine '{name}' property '{prop}' must be a number, got {type(val).__name__} ({val!r})",
                        item=mag,
                        key=prop,
                    )
                if val < 0:
                    _fail(
                        f"Magazine '{name}' property '{prop}' must be a non-negative number (>= 0), got {val}",
                        item=mag,
                        key=prop,
                    )

    # 2. Core checks
    for core in data_section["Cores"]:
        name = core.get("Name", "<unknown>")
        for prop in NON_NEGATIVE_CORE_PROPERTIES:
            if prop in core:
                val = core[prop]
                if not isinstance(val, (int, float)) or isinstance(val, bool):
                    _fail(
                        f"Core '{name}' property '{prop}' must be a number, got {type(val).__name__} ({val!r})",
                        item=core,
                        key=prop,
                    )
                if val < 0:
                    _fail(
                        f"Core '{name}' property '{prop}' must be a non-negative number (>= 0), got {val}",
                        item=core,
                        key=prop,
                    )


# Alias for backward-compatibility
assert_positive_numbers = assert_non_negative_numbers


# ----------------------------------------------------------------------
# Comprehensive Validation and Error Collecting
# ----------------------------------------------------------------------

def validate_dataset(data: Any, allow_suppression: bool = True) -> List[str]:
    """
    Run all assertions on the dataset, collecting and returning a list of all violations.
    Annotates each violation with the exact line number in the dataset JSON file and in DataAssert.py.
    """
    errors: List[str] = []

    # 1. Malformed check
    try:
        assert_not_malformed(data)
    except DataAssertionError as e:
        loc = f"Line {e.json_line} | " if e.json_line else ""
        py_loc = f"DataAssert.py:{e.py_line} | " if e.py_line else ""
        errors.append(f"[{loc}{py_loc}MALFORMED_JSON] {e.raw_message}")
        return errors

    # 2. Unique names & capitalization
    data_section = data["Data"]
    canonical_names: Dict[str, str] = {}
    gun_categories: Dict[str, str] = {}

    for part_type in PART_CATEGORIES:
        seen_in_this_category: Set[str] = set()

        for item in data_section[part_type]:
            name = item["Name"]
            lower_name = name.lower()
            category = item.get("Category", "")

            if lower_name in seen_in_this_category:
                errors.append(
                    _format_error_entry(
                        f"[DUPLICATE_NAME] Category '{part_type}': Duplicate gun name '{name}'",
                        item=item,
                        key="Name",
                    )
                )
            seen_in_this_category.add(lower_name)

            if lower_name in canonical_names:
                expected_name = canonical_names[lower_name]
                if name != expected_name:
                    errors.append(
                        _format_error_entry(
                            f"[INCONSISTENT_CAPITALIZATION] {part_type} '{name}': Capitalization differs from '{expected_name}'",
                            item=item,
                            key="Name",
                        )
                    )
            else:
                canonical_names[lower_name] = name

            if lower_name in gun_categories:
                expected_category = gun_categories[lower_name]
                if category != expected_category:
                    errors.append(
                        _format_error_entry(
                            f"[CATEGORY_MISMATCH] Gun '{name}' in '{part_type}' has category '{category}', expected '{expected_category}'",
                            item=item,
                            key="Category",
                        )
                    )
            else:
                gun_categories[lower_name] = category

    # 3. Keys validation
    core_valid = set(VALID_CORE_KEYS)
    if not allow_suppression:
        core_valid.discard("Suppression")

    part_keys_lower = {k.lower(): k for k in VALID_PART_KEYS}
    mag_keys_lower = {k.lower(): k for k in VALID_MAGAZINE_KEYS}
    core_keys_lower = {k.lower(): k for k in core_valid}

    for part_type in PART_CATEGORIES:
        if part_type == "Magazines":
            valid_keys = VALID_MAGAZINE_KEYS
            lower_lookup = mag_keys_lower
        elif part_type == "Cores":
            valid_keys = core_valid
            lower_lookup = core_keys_lower
        else:
            valid_keys = VALID_PART_KEYS
            lower_lookup = part_keys_lower

        for item in data_section[part_type]:
            name = item.get("Name", "<unknown>")
            for key in item.keys():
                if key.startswith("__"):
                    continue
                if key in valid_keys:
                    continue

                key_lower = key.lower()
                if part_type == "Cores" and key == "Movement_Speed":
                    errors.append(
                        _format_error_entry(
                            f"[INVALID_KEY] Core '{name}': Invalid key 'Movement_Speed'. Expected 'Movement_Speed_Modifier'.",
                            item=item,
                            key=key,
                        )
                    )
                elif key_lower in lower_lookup:
                    canonical = lower_lookup[key_lower]
                    errors.append(
                        _format_error_entry(
                            f"[INCORRECT_KEY_CAPITALIZATION] {part_type} '{name}': Key '{key}' has incorrect capitalization. Expected '{canonical}'.",
                            item=item,
                            key=key,
                        )
                    )
                else:
                    errors.append(
                        _format_error_entry(
                            f"[UNKNOWN_KEY] {part_type} '{name}': Unknown key '{key}'.",
                            item=item,
                            key=key,
                        )
                    )

    # 4. Magazine properties
    for mag in data_section["Magazines"]:
        name = mag.get("Name", "<unknown>")
        if "Magazine_Size" not in mag or mag["Magazine_Size"] is None:
            errors.append(
                _format_error_entry(
                    f"[MISSING_PROPERTY] Magazine '{name}' is missing 'Magazine_Size'",
                    item=mag,
                )
            )
        if "Reload_Time" not in mag or mag["Reload_Time"] is None:
            errors.append(
                _format_error_entry(
                    f"[MISSING_PROPERTY] Magazine '{name}' is missing 'Reload_Time'",
                    item=mag,
                )
            )

    # 5. Non-negative numbers (>= 0)
    for mag in data_section["Magazines"]:
        name = mag.get("Name", "<unknown>")
        for prop in NON_NEGATIVE_MAGAZINE_PROPERTIES:
            if prop in mag:
                val = mag[prop]
                if not isinstance(val, (int, float)) or isinstance(val, bool) or val < 0:
                    errors.append(
                        _format_error_entry(
                            f"[NEGATIVE_STAT] Magazine '{name}': Property '{prop}' must be a non-negative number (>= 0), got {val!r}",
                            item=mag,
                            key=prop,
                        )
                    )

    for core in data_section["Cores"]:
        name = core.get("Name", "<unknown>")
        for prop in NON_NEGATIVE_CORE_PROPERTIES:
            if prop in core:
                val = core[prop]
                if not isinstance(val, (int, float)) or isinstance(val, bool) or val < 0:
                    errors.append(
                        _format_error_entry(
                            f"[NEGATIVE_STAT] Core '{name}': Property '{prop}' must be a non-negative number (>= 0), got {val!r}",
                            item=core,
                            key=prop,
                        )
                    )

    return errors


def assert_all(data: Any, collect_all: bool = False, allow_suppression: bool = True) -> None:
    """
    Run all assertions on the dataset.
    If collect_all is True, collects and reports all errors before raising.
    Otherwise, raises DataAssertionError on the very first assertion failure.
    """
    if collect_all:
        errors = validate_dataset(data, allow_suppression=allow_suppression)
        if errors:
            formatted_errors = "\n".join(f"  - {err}" for err in errors)
            raise DataAssertionError(
                f"Dataset assertions failed with {len(errors)} violation(s):\n{formatted_errors}"
            )
        return

    # Fail fast mode
    assert_not_malformed(data)
    assert_unique_names_and_capitalization(data)
    assert_valid_keys(data, allow_suppression=allow_suppression)
    assert_magazine_properties(data)
    assert_non_negative_numbers(data)


def assert_file(
    file_path: Path | str,
    collect_all: bool = False,
    allow_suppression: bool = True,
) -> None:
    """
    Load a JSON file from disk using LineNumberDecoder and assert its validity.
    """
    path = Path(file_path)
    if not path.exists():
        raise DataAssertionError(f"Dataset file does not exist: {path}")
    if not path.is_file():
        raise DataAssertionError(f"Path is not a regular file: {path}")

    try:
        with open(path, "r", encoding="utf-8") as f:
            content = f.read()
    except Exception as e:
        raise DataAssertionError(f"Failed to read file '{path}': {e}") from e

    try:
        data = json.loads(content, cls=LineNumberDecoder)
    except json.JSONDecodeError as e:
        raise DataAssertionError(
            f"Malformed JSON in file '{path}': {e}",
            json_line=e.lineno,
            py_line=_get_caller_py_line(),
        ) from e

    assert_all(data, collect_all=collect_all, allow_suppression=allow_suppression)


# ----------------------------------------------------------------------
# CLI Interface
# ----------------------------------------------------------------------

def main() -> int:
    parser = argparse.ArgumentParser(
        description="Run assertions on WeirdGunGameCalc dataset (Data/FullData.json)."
    )
    parser.add_argument(
        "file",
        nargs="?",
        default=str(Path(__file__).parent / "Data" / "FullData.json"),
        help="Path to the dataset JSON file (defaults to Data/FullData.json).",
    )
    parser.add_argument(
        "--collect",
        "--all-errors",
        action="store_true",
        help="Collect and display all assertion failures rather than failing on the first one.",
    )
    parser.add_argument(
        "--strict",
        action="store_true",
        help="Strict mode: disallow optional dataset keys like 'Suppression'.",
    )
    parser.add_argument(
        "--quiet",
        "-q",
        action="store_true",
        help="Suppress progress outputs and only display errors.",
    )

    args = parser.parse_args()
    file_path = Path(args.file)

    if not args.quiet:
        print(f"Asserting dataset: {file_path}")

    try:
        assert_file(
            file_path,
            collect_all=args.collect,
            allow_suppression=not args.strict,
        )
        if not args.quiet:
            print("All dataset assertions passed successfully! ✅")
        return 0

    except DataAssertionError as e:
        print(f"\n❌ Assertion Failed:\n{e}", file=sys.stderr)
        return 1
    except Exception as e:
        print(f"\n❌ Unexpected error: {e}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())

