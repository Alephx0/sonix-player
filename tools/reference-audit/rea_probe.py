"""Reproduce read-only REA MCP inspection; never launches the target executable."""
import json
import os
from pathlib import Path
import subprocess
import sys


def main():
    target = Path(sys.argv[1]).resolve()
    output = Path(sys.argv[2]).resolve()
    output.mkdir(parents=True, exist_ok=True)
    env = dict(os.environ, REA_MCP_OUTPUT_SCHEMA_PROFILE="off")
    command = ["cmd.exe", "/d", "/c", "npx", "-y", "rea-agents@6.7.0", "mcp"]
    with (output / "rea-mcp-stderr.log").open("w", encoding="utf-8") as errors:
        process = subprocess.Popen(command, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                   stderr=errors, text=True, encoding="utf-8", env=env)
        def rpc(method, params, ident):
            request = {"jsonrpc": "2.0", "method": method, "params": params}
            if ident is not None:
                request["id"] = ident
            process.stdin.write(json.dumps(request) + "\n")
            process.stdin.flush()
            if ident is None:
                return
            for line in process.stdout:
                response = json.loads(line)
                if response.get("id") == ident:
                    return response
            raise RuntimeError("REA exited before response")
        try:
            init = rpc("initialize", {"protocolVersion": "2024-11-05", "capabilities": {},
                       "clientInfo": {"name": "sonix-readonly-audit", "version": "1.0"}}, 1)
            (output / "rea-initialize.json").write_text(json.dumps(init, indent=2), encoding="utf-8")
            rpc("notifications/initialized", {}, None)
            catalog = rpc("tools/list", {}, 2)
            all_tools = catalog["result"]["tools"]
            cursor = catalog["result"].get("nextCursor")
            page_id = 100
            while cursor:
                page = rpc("tools/list", {"cursor": cursor}, page_id)
                all_tools.extend(page["result"]["tools"])
                cursor = page["result"].get("nextCursor")
                page_id += 1
            wanted = {"inspect_go_binary", "inspect_desktop_application"}
            selected = [t for t in all_tools if t["name"] in wanted]
            (output / "rea-tool-names.json").write_text(json.dumps([t["name"] for t in all_tools], indent=2), encoding="utf-8")
            (output / "rea-selected-tools.json").write_text(json.dumps(selected, indent=2), encoding="utf-8")
            for ident, tool in enumerate(selected, 3):
                name = tool["name"]
                result = rpc("tools/call", {"name": name, "arguments": {"path": str(target)}}, ident)
                (output / (name + ".json")).write_text(json.dumps(result, indent=2), encoding="utf-8")
                print(name, "completed", "error" if result.get("error") else "response saved")
        finally:
            process.stdin.close()
            try:
                process.wait(timeout=15)
            except subprocess.TimeoutExpired:
                process.terminate()


if __name__ == "__main__":
    main()
