"""
Pre-compilation script for platformio.ini.
Edit .env if you wish to change the values.
"""

from dataclasses import dataclass
import os
Import("env")

try:
    from dotenv import load_dotenv
except ImportError:
    env.Execute("$PYTHONEXE -m pip install python-dotenv")


@dataclass
class Config:
    """Class for environment variables"""
    WIFI_SSID: str = ""
    WIFI_PWD: str = ""


"""Config header file name"""
CONFIG_HEADER = "config.hpp"

"""
Config header file contents template.
Values to be added by format().
"""
CONFIG_HEADER_CONTENTS = """
// AUTO GENERATED FILE, DO NOT EDIT
#ifndef CONFIG_HPP
#define CONFIG_HPP

namespace Zotbins
{{
    namespace Config
    {{
        const char *WIFI_SSID = \"{WIFI_SSID}\";
        const char *WIFI_PWD = \"{WIFI_PWD}\";
    }}
}}
#endif
"""


def loadConfig() -> Config:
    """
    Load configuration from .env file.
    If .env does not exist, return default values.

    Returns:
    Config object with values of enviromental variables
    """

    # Return default if .env does not exist
    if not os.path.exists(".env"):
        return Config()

    # Load enviromental variables
    load_dotenv()
    WIFI_SSID = os.environ.get("ZBIN_CORE_WIFI_SSID", "")
    WIFI_PWD = os.environ.get("ZBIN_CORE_WIFI_PWD", "")

    return Config(WIFI_SSID, WIFI_PWD)


def createConfigHeader(config: Config) -> None:
    """
    Generates config.hpp in the include/ directory.
    Uses the values in config to set values in header.

    Parameters:
    config (Config) Config object with values of environmental variables
    """

    if os.path.exists("include/"):
        CONFIG_HEADER_PATH = "include" + os.sep + CONFIG_HEADER

        # Write to header file
        with open(CONFIG_HEADER_PATH, "w+") as file:
            file.write(CONFIG_HEADER_CONTENTS.format(
                WIFI_SSID=config.WIFI_SSID,
                WIFI_PWD=config.WIFI_PWD,
            ))
    else:
        raise FileNotFoundError("include/ directory not found")


# Load config and create config header file
config = loadConfig()
createConfigHeader(config)
