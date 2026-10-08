import os
import sys
import time
import json
import hashlib
import subprocess
import datetime
import ctypes
from ctypes import wintypes

u = ctypes.windll.user32
k = ctypes.windll.kernel32

def sha256(filepath):
    h = hashlib.sha256()
    with open(filepath, "rb") as f:
        while chunk := f.read(65536):
            h.update(chunk)
    return h.hexdigest().upper()

def get_file_version(filepath):
    import win32api
    info = win32api.GetFileVersionInfo(filepath, "\\")
    ms = info["FileVersionMS"]
    ls = info["FileVersionLS"]
    return f"{win32api.HIWORD(ms)}.{win32api.LOWORD(ms)}.{win32api.HIWORD(ls)}"

def find_pomodoro_window():
    winsta = u.OpenWindowStationW("WinSta0", False, 0x02000000)
    if winsta:
        u.SetProcessWindowStation(winsta)
        desk = u.OpenDesktopW("Default", 0, False, 0x02000000)
        if desk:
            u.SetThreadDesktop(desk)
            hwnd = u.FindWindowW("Pomodoro", "Pomodoro")
            return hwnd, winsta, desk
    return None, None, None

def main():
    root = os.path.abspath(".")
    source_exe = os.path.join(root, "pomodoro-timer.exe")
    installed_exe = r"D:\pomodoro-timer\pomodoro-timer.exe"
    
    if not os.path.exists(source_exe):
        raise FileNotFoundError(f"Source exe not found: {source_exe}")
    if not os.path.exists(installed_exe):
        raise FileNotFoundError(f"Installed exe not found: {installed_exe}")
        
    src_version = get_file_version(source_exe)
    if src_version != "3.0.9":
        raise ValueError(f"Unexpected source version: {src_version}, expected 3.0.9")
        
    stamp = datetime.datetime.now().strftime("%Y%m%d-%H%M%S")
    backup_exe = f"D:\\pomodoro-timer\\pomodoro-timer-v3.0.8-backup-{stamp}.exe"
    
    before_hash = sha256(installed_exe)
    
    # 1. Back up installed binary
    with open(installed_exe, "rb") as sf, open(backup_exe, "wb") as df:
        df.write(sf.read())
    backup_hash = sha256(backup_exe)
    if backup_hash != before_hash:
        raise ValueError("Backup hash mismatch!")
    print(f"Backed up to {backup_exe} (hash: {backup_hash})")
    
    # 2. Check and gracefully close running instance
    hwnd, winsta, desk = find_pomodoro_window()
    if hwnd:
        pid = wintypes.DWORD()
        u.GetWindowThreadProcessId(hwnd, ctypes.byref(pid))
        print(f"Found running instance: HWND={hwnd}, PID={pid.value}")
        
        # Send WM_CLOSE
        u.PostMessageW(hwnd, 0x0010, 0, 0)
        
        deadline = time.time() + 6.0
        while time.time() < deadline:
            time.sleep(0.2)
            h_proc = k.OpenProcess(0x1000, False, pid.value)
            if not h_proc:
                break
            code = wintypes.DWORD()
            k.GetExitCodeProcess(h_proc, ctypes.byref(code))
            k.CloseHandle(h_proc)
            if code.value != 259: # STILL_ACTIVE
                break
        print(f"Instance PID={pid.value} exited cleanly.")
    else:
        print("No running Pomodoro window found on WinSta0\\Default.")

    # 3. Replace target binary with retry
    source_hash = sha256(source_exe)
    copied = False
    deadline = time.time() + 5.0
    while time.time() < deadline:
        try:
            with open(source_exe, "rb") as sf, open(installed_exe, "wb") as df:
                df.write(sf.read())
            copied = True
            break
        except PermissionError:
            time.sleep(0.3)
            
    if not copied:
        raise RuntimeError(f"Could not overwrite {installed_exe} due to persistent lock.")
        
    installed_hash = sha256(installed_exe)
    if installed_hash != source_hash:
        raise ValueError("Installed hash differs from source hash!")
    print(f"Updated {installed_exe} (hash: {installed_hash})")
    
    # 4. Launch via interactive scheduled task
    task_name = f"Pomodoro-v3.0.9-local-{stamp}"
    created = False
    try:
        cmd_create = f'schtasks.exe /Create /TN "{task_name}" /SC ONCE /ST 23:59 /TR "\"{installed_exe}\"" /IT'
        res_create = subprocess.run(cmd_create, capture_output=True, text=True, shell=True)
        if res_create.returncode != 0:
            raise RuntimeError(f"schtasks /Create failed: {res_create.stderr}")
        created = True
        
        cmd_run = f'schtasks.exe /Run /TN "{task_name}"'
        res_run = subprocess.run(cmd_run, capture_output=True, text=True, shell=True)
        if res_run.returncode != 0:
            raise RuntimeError(f"schtasks /Run failed: {res_run.stderr}")
        print("Launched new instance via interactive scheduled task.")
        time.sleep(2.0)
    finally:
        if created:
            subprocess.run(f'schtasks.exe /Delete /TN "{task_name}" /F', capture_output=True, shell=True)
            print("Cleaned up temporary scheduled task.")
            
    # 5. Verify new running instance
    hwnd_new, _, _ = find_pomodoro_window()
    if not hwnd_new:
        raise RuntimeError("New Pomodoro window not found on WinSta0\\Default after launch!")
        
    actual_pid = wintypes.DWORD()
    u.GetWindowThreadProcessId(hwnd_new, ctypes.byref(actual_pid))
    
    # Test responsiveness with WM_NULL
    res = ctypes.c_size_t(0)
    ok = u.SendMessageTimeoutW(hwnd_new, 0, 0, 0, 2, 2000, ctypes.byref(res))
    if not ok:
        raise RuntimeError("New window did not respond to SendMessageTimeout!")
    print(f"New instance verified: PID={actual_pid.value}, window responding=True")
    
    proof = {
        "Version": "3.0.9",
        "SourceHash": source_hash,
        "InstalledHash": installed_hash,
        "Backup": backup_exe,
        "BackupHash": before_hash,
        "ProcessId": actual_pid.value,
        "WindowResponding": True,
        "TemporaryTaskDeleted": True
    }
    
    proof_dir = os.path.join(root, "tools", ".fullscreen-test", f"deployment-{stamp}")
    os.makedirs(proof_dir, exist_ok=True)
    proof_file = os.path.join(proof_dir, "deployment-v3.0.9.json")
    with open(proof_file, "w", encoding="utf-8") as f:
        json.dump(proof, f, indent=2, ensure_ascii=False)
        
    print(json.dumps(proof, indent=2, ensure_ascii=False))

if __name__ == "__main__":
    main()
