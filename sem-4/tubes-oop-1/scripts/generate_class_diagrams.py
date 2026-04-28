import re
import os
import sys

def main():
    if len(sys.argv) < 2:
        print("Usage: python split_puml.py <all_classes.puml>")
        sys.exit(1)
        
    input_file = sys.argv[1]
    
    try:
        with open(input_file, "r") as f:
            content = f.read()
            
        content = re.sub(
            r'@startuml.*?(?=/\' Objects \')',
            "@startuml\nskinparam classAttributeIconSize 0\ntop to bottom direction\nskinparam nodesep 150\nskinparam ranksep 250\nskinparam padding 10\nskinparam backgroundcolor transparent\n\n\n",
            content,
            flags=re.DOTALL
        )
        with open(input_file, "w") as f:
            f.write(content)
                
        lines = content.splitlines(True)
    except FileNotFoundError:
        print(f"Error: {input_file} not found.")
        sys.exit(1)

    objects = {}
    current_class = None
    current_block = []

    # Regex definitions
    class_pattern = re.compile(r'^(abstract\s+class|class|enum\s+class|enum|interface)\s+([A-Za-z0-9_]+)')
    
    # 1. Parse individual class blocks
    in_objects_section = True
    for line in lines:
        if "/' Inheritance relationships '/" in line:
            in_objects_section = False
            
        if in_objects_section:
            match = class_pattern.match(line.strip())
            if match:
                current_class = match.group(2)
                current_block = [line]
            elif current_class:
                current_block.append(line)
                if line.strip() == "}":
                    objects[current_class] = "".join(current_block)
                    current_class = None

    inheritances = []
    collaborations = []

    # 2. Parse relationships
    for line in lines:
        line_clean = line.strip()
        if not line_clean or line_clean.startswith("/'"):
            continue
            
        if "<|--" in line_clean:
            inheritances.append(line_clean)
        elif any(op in line_clean for op in ["*--", "o--", "-->", "<--", "--*", "--o"]):
            collaborations.append(line_clean)

    # Helper function to extract class names from relationship lines
    def extract_names(rel_line, op_list):
        for op in op_list:
            if op in rel_line:
                parts = rel_line.split(op)
                if len(parts) == 2:
                    left = re.sub(r'"[^"]*"', '', parts[0]).strip()
                    right = re.sub(r'"[^"]*"', '', parts[1]).strip()
                    left = left.split()[0] if left else ""
                    right = right.split()[0] if right else ""
                    return left, op, right
        return None, None, None

    # Setup directories
    os.makedirs("doc/diagrams/inheritance", exist_ok=True)
    os.makedirs("doc/diagrams/collaboration", exist_ok=True)

    print("Generating per-class inheritance diagrams...")
    # Generate Inheritance Diagrams
    for cls_name, cls_block in objects.items():
        related_classes = {cls_name}
        related_rels = []
        
        for rel in inheritances:
            left, op, right = extract_names(rel, ["<|--"])
            if left == cls_name or right == cls_name:
                if left: related_classes.add(left)
                if right: related_classes.add(right)
                related_rels.append(rel)
                
        if len(related_classes) > 1: # Only if it inherits or is inherited
            with open(f"doc/diagrams/inheritance/{cls_name}_inherit.puml", "w") as f:
                f.write("@startuml\n\n")
                f.write("!theme plain\n")
                f.write("skinparam classBackgroundColor transparent\n")
                f.write("skinparam classAttributeIconSize 0\n\n")
                for rel_cls in related_classes:
                    if rel_cls in objects:
                        f.write(objects[rel_cls] + "\n")
                    else:
                        f.write(f"class {rel_cls}\n\n")
                f.write("\n/' Inheritance links '/\n")
                for rel in related_rels:
                    f.write(rel + "\n")
                f.write("\n@enduml\n")

    print("Generating per-class collaboration diagrams...")
    # Generate Collaboration Diagrams
    for cls_name, cls_block in objects.items():
        related_classes = {cls_name}
        related_rels = []
        
        # Add collaboration links natively
        for rel in collaborations:
            left, op, right = extract_names(rel, ["*--", "o--", "-->", "<--", "--*", "--o"])
            if left == cls_name or right == cls_name:
                if left: related_classes.add(left)
                if right: related_classes.add(right)
                related_rels.append(rel)
                
        if len(related_classes) > 1: # Only if it collaborates or aggregates
            with open(f"doc/diagrams/collaboration/{cls_name}_collab.puml", "w") as f:
                f.write("@startuml\n\n")
                f.write("!theme plain\n")
                f.write("skinparam classBackgroundColor transparent\n")
                f.write("skinparam classAttributeIconSize 0\n\n")
                for rel_cls in related_classes:
                    if rel_cls in objects:
                        f.write(objects[rel_cls] + "\n")
                    else:
                        f.write(f"class {rel_cls}\n\n")
                f.write("\n/' Collaboration links '/\n")
                for rel in related_rels:
                    f.write(rel + "\n")
                f.write("\n@enduml\n")
                
    print("Done handling per-class PlantUML scripts.")

if __name__ == "__main__":
    main()
