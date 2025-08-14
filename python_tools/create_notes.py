# create_notes.py
import os

def aggregate_source_files():
    """
    Prompts the user for a subdirectory within 'src', then recursively finds all
    .h and .cpp files and copies their contents into a single text file.
    """
    # --- Configuration ---
    PROJECT_ROOT_SUBDIR = "src"
    OUTPUT_FILENAME = "project_notes.txt"
    VALID_EXTENSIONS = ('.h', '.cpp')
    
    print("This script will aggregate .h and .cpp files into a single text file.")
    
    # 1. Get user input for the target directory
    target_subdir = input(f"Enter the directory to search within '{PROJECT_ROOT_SUBDIR}/': ")
    
    # 2. Construct and validate the full path
    # This assumes the script is run from the project root directory.
    search_path = os.path.join(PROJECT_ROOT_SUBDIR, target_subdir)
    
    if not os.path.isdir(search_path):
        print(f"\n[ERROR] Directory not found: '{search_path}'")
        print("Please make sure you are running this script from your project's root directory (e.g., 'SalixGameStudio').")
        return

    print(f"\nSearching in: '{os.path.abspath(search_path)}'")
    
    found_files_count = 0
    
    # 3. Open the output file and start processing
    try:
        with open(OUTPUT_FILENAME, 'w', encoding='utf-8') as f_out:
            # 4. Walk the directory tree recursively
            for dirpath, _, filenames in os.walk(search_path):
                for filename in filenames:
                    # 5. Check for valid file extensions
                    if filename.endswith(VALID_EXTENSIONS):
                        found_files_count += 1
                        file_path = os.path.join(dirpath, filename)
                        
                        # Get a clean, relative path for the header
                        relative_path = os.path.relpath(file_path).replace('\\', '/')
                        print(f"  -> Copying '{relative_path}'...")
                        
                        # 6. Write a clear header for each file to the output
                        f_out.write(f"// =================================================================================\n")
                        f_out.write(f"// Filename:    {relative_path}\n")
                        f_out.write(f"// =================================================================================\n\n")
                        
                        # 7. Read the source file and write its content to the output
                        try:
                            with open(file_path, 'r', encoding='utf-8', errors='ignore') as f_in:
                                f_out.write(f_in.read())
                                f_out.write("\n\n") # Add spacing between files for readability
                        except Exception as e:
                            error_message = f"// ERROR: Could not read file. Reason: {e}\n\n"
                            f_out.write(error_message)
                            print(f"  [WARNING] Could not read file '{relative_path}': {e}")
                            
        print(f"\nSuccess! Found and aggregated {found_files_count} files.")
        print(f"Output saved to: '{os.path.abspath(OUTPUT_FILENAME)}'")

    except IOError as e:
        print(f"\n[ERROR] Could not write to output file '{OUTPUT_FILENAME}': {e}")

if __name__ == "__main__":
    aggregate_source_files()