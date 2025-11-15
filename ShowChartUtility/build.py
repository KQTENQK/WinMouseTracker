import PyInstaller.__main__
import sys
import os
import matplotlib
import glob
import tkinter as tk

def get_tkinter_paths():
    """Find path Tcl/Tk"""
    try:
        root = tk.Tk()
        
        tcl_library = root.tk.exprstring('$tcl_library')
        tk_library = root.tk.exprstring('$tk_library')
        
        root.destroy()
        
        return tcl_library, tk_library
    except Exception as e:
        print(f"Error getting Tkinter paths: {e}")
        return None, None

def find_tcl_tk_dlls():
    """Find DLL Tcl и Tk"""
    python_dir = os.path.dirname(sys.executable)
    dlls_dir = os.path.join(python_dir, "DLLs")
    
    tcl_dll = None
    tk_dll = None
    
    search_paths = [
        dlls_dir,
        os.path.join(python_dir, "Library", "bin"),
        os.path.join(os.path.dirname(python_dir), "DLLs"),
    ]
    
    for path in search_paths:
        if not os.path.exists(path):
            continue
            
        tcl_patterns = ["tcl*.dll", "tcl*.*.dll"]

        for pattern in tcl_patterns:
            matches = glob.glob(os.path.join(path, pattern))
            if matches:
                tcl_dll = matches[0]
                break
                
        tk_patterns = ["tk*.dll", "tk*.*.dll"]

        for pattern in tk_patterns:
            matches = glob.glob(os.path.join(path, pattern))
            if matches:
                tk_dll = matches[0]
                break
                
        if tcl_dll and tk_dll:
            break
    
    return tcl_dll, tk_dll

def main():
    tcl_library, tk_library = get_tkinter_paths()

    print(f"Tcl library path: {tcl_library}")
    print(f"Tk library path: {tk_library}")
    
    tcl_dll, tk_dll = find_tcl_tk_dlls()
    
    print(f"Tcl DLL: {tcl_dll}")
    print(f"Tk DLL: {tk_dll}")

    mpl_data_dir = matplotlib.get_data_path()
    
    cmd = [
        "show_2d_points.py",
        "--onefile",
        "--console",
        "--name=Show2dPoints",
        "--hidden-import=_tkinter",
        "--hidden-import=numpy",
        "--hidden-import=matplotlib",
        "--hidden-import=matplotlib.backends.backend_agg",
        "--hidden-import=matplotlib.backends.backend_tkagg",
        "--hidden-import=PIL._tkinter_finder",
    ]

    if tcl_library and os.path.exists(tcl_library):
        cmd.append(f"--add-data={tcl_library};tcl")
    if tk_library and os.path.exists(tk_library):
        cmd.append(f"--add-data={tk_library};tk")
    if tcl_dll and os.path.exists(tcl_dll):
        cmd.append(f"--add-binary={tcl_dll};.")
    if tk_dll and os.path.exists(tk_dll):
        cmd.append(f"--add-binary={tk_dll};.")

    cmd.append(f"--add-data={mpl_data_dir};matplotlib/mpl-data")
    
    print("Building with command:", " ".join(cmd))
    PyInstaller.__main__.run(cmd)

if __name__ == "__main__":
    main()