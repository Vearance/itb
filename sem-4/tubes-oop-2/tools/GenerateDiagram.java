import com.sun.source.tree.ArrayTypeTree;
import com.sun.source.tree.ClassTree;
import com.sun.source.tree.CompilationUnitTree;
import com.sun.source.tree.IdentifierTree;
import com.sun.source.tree.ImportTree;
import com.sun.source.tree.IntersectionTypeTree;
import com.sun.source.tree.MemberSelectTree;
import com.sun.source.tree.ParameterizedTypeTree;
import com.sun.source.tree.PrimitiveTypeTree;
import com.sun.source.tree.Tree;
import com.sun.source.tree.Tree.Kind;
import com.sun.source.tree.WildcardTree;
import com.sun.source.util.JavacTask;
import com.sun.source.util.TreePath;
import com.sun.source.util.TreePathScanner;
import javax.lang.model.element.Modifier;
import javax.tools.JavaCompiler;
import javax.tools.JavaFileObject;
import javax.tools.StandardJavaFileManager;
import javax.tools.ToolProvider;
import java.io.IOException;
import java.io.PrintWriter;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Deque;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.Set;
import java.util.TreeMap;
import java.util.TreeSet;
import java.util.stream.Collectors;
import java.util.stream.Stream;

public final class GenerateDiagram {
    private static final Path DEFAULT_SOURCE_ROOT = Path.of("src/main/java");
    private static final Path DEFAULT_OUTPUT_FILE = Path.of("docs/uml/class-diagram.puml");

    public static void main(String[] args) throws Exception {
        Path sourceRoot = args.length > 0 ? Path.of(args[0]) : DEFAULT_SOURCE_ROOT;
        Path outputFile = args.length > 1 ? Path.of(args[1]) : DEFAULT_OUTPUT_FILE;

        List<Path> sourceFiles = collectJavaFiles(sourceRoot);
        if (sourceFiles.isEmpty()) {
            throw new IllegalStateException("No Java sources found under " + sourceRoot);
        }

        JavaCompiler compiler = ToolProvider.getSystemJavaCompiler();
        if (compiler == null) {
            throw new IllegalStateException("A JDK is required to run this generator.");
        }

        DiagramModel model = new DiagramModel();
        try (StandardJavaFileManager fileManager = compiler.getStandardFileManager(null, null, StandardCharsets.UTF_8)) {
            Iterable<? extends JavaFileObject> javaFiles = fileManager.getJavaFileObjectsFromPaths(sourceFiles);
            JavacTask task = (JavacTask) compiler.getTask(null, fileManager, null, List.of("-proc:none"), null, javaFiles);
            Iterable<? extends CompilationUnitTree> compilationUnits = task.parse();

            SourceScanner scanner = new SourceScanner(model);
            for (CompilationUnitTree unit : compilationUnits) {
                scanner.scan(unit, null);
            }
        }

        model.write(outputFile);
        System.out.println("Wrote " + outputFile + " with " + model.typeCount() + " types and " + model.relationCount() + " relations.");
    }

    private static List<Path> collectJavaFiles(Path sourceRoot) throws IOException {
        try (Stream<Path> stream = Files.walk(sourceRoot)) {
            return stream
                    .filter(Files::isRegularFile)
                    .filter(path -> path.toString().endsWith(".java"))
                    .sorted()
                    .collect(Collectors.toList());
        }
    }

    private static String joinPath(List<String> parts) {
        return String.join(".", parts);
    }

    private static String sanitizeAlias(String fqn) {
        return "type_" + fqn.replaceAll("[^A-Za-z0-9]+", "_");
    }

    private static String indent(int depth) {
        return "  ".repeat(depth);
    }

    private static String renderTypeName(Tree tree) {
        if (tree == null) {
            return "";
        }

        return switch (tree.getKind()) {
            case PARAMETERIZED_TYPE -> renderTypeName(((ParameterizedTypeTree) tree).getType());
            case MEMBER_SELECT -> renderMemberSelect((MemberSelectTree) tree);
            case IDENTIFIER -> ((IdentifierTree) tree).getName().toString();
            case ARRAY_TYPE -> renderTypeName(((ArrayTypeTree) tree).getType());
            case PRIMITIVE_TYPE -> ((PrimitiveTypeTree) tree).getPrimitiveTypeKind().name().toLowerCase(Locale.ROOT);
            case UNBOUNDED_WILDCARD -> "?";
            case EXTENDS_WILDCARD, SUPER_WILDCARD -> renderTypeName(((WildcardTree) tree).getBound());
            case INTERSECTION_TYPE -> renderIntersectionType((IntersectionTypeTree) tree);
            default -> tree.toString();
        };
    }

    private static String renderMemberSelect(MemberSelectTree tree) {
        String prefix = renderTypeName(tree.getExpression());
        if (prefix.isBlank()) {
            return tree.getIdentifier().toString();
        }
        return prefix + "." + tree.getIdentifier();
    }

    private static String renderIntersectionType(IntersectionTypeTree tree) {
        List<? extends Tree> bounds = tree.getBounds();
        return bounds.isEmpty() ? tree.toString() : renderTypeName(bounds.get(0));
    }

    private static TypeKind classify(ClassTree tree) {
        return switch (tree.getKind()) {
            case INTERFACE -> TypeKind.INTERFACE;
            case ENUM -> TypeKind.ENUM;
            case RECORD -> TypeKind.RECORD;
            case ANNOTATION_TYPE -> TypeKind.ANNOTATION;
            default -> TypeKind.CLASS;
        };
    }

    private static String renderTypeDeclaration(TypeNode type) {
        String alias = sanitizeAlias(type.fqn());
        return switch (type.kind()) {
            case INTERFACE -> "interface \"" + type.displayName() + "\" as " + alias;
            case ENUM -> "enum \"" + type.displayName() + "\" as " + alias;
            case RECORD -> "class \"" + type.displayName() + "\" as " + alias + " <<record>>";
            case ANNOTATION -> "annotation \"" + type.displayName() + "\" as " + alias;
            case CLASS -> type.abstractType()
                    ? "abstract class \"" + type.displayName() + "\" as " + alias
                    : "class \"" + type.displayName() + "\" as " + alias;
        };
    }

    private static final class SourceScanner extends TreePathScanner<Void, Void> {
        private final DiagramModel model;
        private String currentPackage = "";
        private Map<String, String> explicitImports = Map.of();
        private List<String> starImports = List.of();
        private final Deque<String> typeStack = new ArrayDeque<>();

        private SourceScanner(DiagramModel model) {
            this.model = model;
        }

        @Override
        public Void visitCompilationUnit(CompilationUnitTree tree, Void unused) {
            currentPackage = tree.getPackageName() == null ? "" : tree.getPackageName().toString();
            Map<String, String> imports = new LinkedHashMap<>();
            List<String> starPackages = new ArrayList<>();

            for (ImportTree importTree : tree.getImports()) {
                if (importTree.isStatic()) {
                    continue;
                }

                String importedName = importTree.getQualifiedIdentifier().toString();
                if (importedName.endsWith(".*")) {
                    String packageName = importedName.endsWith(".*") ? importedName.substring(0, importedName.length() - 2) : importedName;
                    starPackages.add(packageName);
                } else {
                    imports.put(simpleName(importedName), importedName);
                }
            }

            explicitImports = imports;
            starImports = starPackages;
            return super.visitCompilationUnit(tree, unused);
        }

        @Override
        public Void visitClass(ClassTree tree, Void unused) {
            TreePath parentPath = getCurrentPath().getParentPath();
            Tree parentLeaf = parentPath == null ? null : parentPath.getLeaf();
            if (!(parentLeaf instanceof CompilationUnitTree || parentLeaf instanceof ClassTree)) {
                return null;
            }

            String simpleName = tree.getSimpleName().toString();
            List<String> enclosingTypes = List.copyOf(typeStack);
            List<String> sourceTypePath = new ArrayList<>(typeStack);
            sourceTypePath.add(simpleName);
            String fqn = qualify(currentPackage, sourceTypePath);

            TypeNode type = new TypeNode(
                    fqn,
                    currentPackage,
                    enclosingTypes,
                    simpleName,
                    joinPath(sourceTypePath),
                    classify(tree),
                    tree.getModifiers().getFlags().contains(Modifier.ABSTRACT)
            );

            model.addType(type);
            model.addPendingRelations(collectRelations(tree, fqn, sourceTypePath));

            typeStack.addLast(simpleName);
            try {
                return super.visitClass(tree, unused);
            } finally {
                typeStack.removeLast();
            }
        }

        private List<PendingRelation> collectRelations(ClassTree tree, String sourceFqn, List<String> sourceTypePath) {
            List<PendingRelation> relations = new ArrayList<>();

            if (tree.getKind() == Kind.INTERFACE || tree.getKind() == Kind.ANNOTATION_TYPE) {
                for (Tree target : tree.getImplementsClause()) {
                    relations.add(new PendingRelation(
                            sourceFqn,
                            currentPackage,
                            List.copyOf(sourceTypePath),
                            RelationKind.EXTENDS,
                            renderTypeName(target),
                            Map.copyOf(explicitImports),
                            List.copyOf(starImports)
                    ));
                }
                return relations;
            }

            if (tree.getExtendsClause() != null) {
                relations.add(new PendingRelation(
                        sourceFqn,
                        currentPackage,
                        List.copyOf(sourceTypePath),
                        RelationKind.EXTENDS,
                        renderTypeName(tree.getExtendsClause()),
                        Map.copyOf(explicitImports),
                        List.copyOf(starImports)
                ));
            }

            for (Tree target : tree.getImplementsClause()) {
                relations.add(new PendingRelation(
                        sourceFqn,
                        currentPackage,
                        List.copyOf(sourceTypePath),
                        RelationKind.IMPLEMENTS,
                        renderTypeName(target),
                        Map.copyOf(explicitImports),
                        List.copyOf(starImports)
                ));
            }

            return relations;
        }

        private static String simpleName(String qualifiedName) {
            int lastDot = qualifiedName.lastIndexOf('.');
            return lastDot < 0 ? qualifiedName : qualifiedName.substring(lastDot + 1);
        }

        private static String qualify(String packageName, Collection<String> sourceTypePath) {
            String path = joinPath(new ArrayList<>(sourceTypePath));
            if (packageName == null || packageName.isBlank()) {
                return path;
            }
            return packageName + "." + path;
        }
    }

    private static final class DiagramModel {
        private final PackageNode root = new PackageNode("");
        private final Map<String, TypeNode> typesByFqn = new LinkedHashMap<>();
        private final Map<String, List<TypeNode>> typesBySimpleName = new HashMap<>();
        private final List<PendingRelation> pendingRelations = new ArrayList<>();

        void addType(TypeNode type) {
            if (typesByFqn.putIfAbsent(type.fqn(), type) != null) {
                return;
            }

            typesBySimpleName.computeIfAbsent(type.simpleName(), key -> new ArrayList<>()).add(type);
            PackageNode node = root;
            if (!type.packageName().isBlank()) {
                for (String segment : type.packageName().split("\\.")) {
                    node = node.children.computeIfAbsent(segment, PackageNode::new);
                }
            }
            node.types.add(type);
        }

        void addPendingRelations(List<PendingRelation> relations) {
            pendingRelations.addAll(relations);
        }

        int typeCount() {
            return typesByFqn.size();
        }

        int relationCount() {
            return resolveRelations().size();
        }

        void write(Path outputFile) throws IOException {
            Files.createDirectories(Objects.requireNonNull(outputFile.getParent(), "output file must have a parent directory"));

            List<String> lines = new ArrayList<>();
            lines.add("@startuml");
            lines.add("title Banana Republic master class diagram");
            lines.add("skinparam packageStyle rectangle");
            lines.add("skinparam classAttributeIconSize 0");
            lines.add("hide empty members");
            lines.add("");
            writePackageTree(lines, root, 0);

            List<String> relations = resolveRelations();
            if (!relations.isEmpty()) {
                lines.add("");
                lines.addAll(relations);
            }

            lines.add("@enduml");

            try (PrintWriter writer = new PrintWriter(Files.newBufferedWriter(outputFile, StandardCharsets.UTF_8))) {
                for (String line : lines) {
                    writer.println(line);
                }
            }
        }

        private void writePackageTree(List<String> lines, PackageNode node, int depth) {
            if (!node.types.isEmpty()) {
                List<TypeNode> sortedTypes = new ArrayList<>(node.types);
                sortedTypes.sort((left, right) -> left.displayName().compareToIgnoreCase(right.displayName()));
                for (TypeNode type : sortedTypes) {
                    lines.add(indent(depth) + renderTypeDeclaration(type));
                }
                if (!node.children.isEmpty()) {
                    lines.add("");
                }
            }

            boolean firstChild = true;
            for (PackageNode child : node.children.values()) {
                if (!firstChild) {
                    lines.add("");
                }
                firstChild = false;
                lines.add(indent(depth) + "package " + child.segmentName + " {");
                writePackageTree(lines, child, depth + 1);
                lines.add(indent(depth) + "}");
            }
        }

        private List<String> resolveRelations() {
            Set<String> relationLines = new TreeSet<>();
            for (PendingRelation relation : pendingRelations) {
                Optional<String> targetFqn = resolveTypeReference(relation);
                if (targetFqn.isEmpty()) {
                    continue;
                }

                TypeNode source = typesByFqn.get(relation.sourceFqn());
                TypeNode target = typesByFqn.get(targetFqn.get());
                if (source == null || target == null) {
                    continue;
                }

                String arrow = relation.relationKind() == RelationKind.EXTENDS ? "--|>" : "..|>";
                relationLines.add(sanitizeAlias(source.fqn()) + " " + arrow + " " + sanitizeAlias(target.fqn()));
            }
            return new ArrayList<>(relationLines);
        }

        private Optional<String> resolveTypeReference(PendingRelation relation) {
            String reference = relation.rawTarget().trim();
            if (reference.isEmpty()) {
                return Optional.empty();
            }

            Optional<String> exact = findKnownType(reference);
            if (exact.isPresent()) {
                return exact;
            }

            if (reference.contains(".")) {
                String packagePrefixed = relation.sourcePackage().isBlank()
                        ? reference
                        : relation.sourcePackage() + "." + reference;
                Optional<String> prefixed = findKnownType(packagePrefixed);
                if (prefixed.isPresent()) {
                    return prefixed;
                }
            }

            List<String> sourceTypePath = relation.sourceTypePath();
            for (int prefixSize = sourceTypePath.size(); prefixSize >= 1; prefixSize--) {
                String prefix = joinPath(sourceTypePath.subList(0, prefixSize));
                String candidate = relation.sourcePackage().isBlank()
                        ? prefix + "." + reference
                        : relation.sourcePackage() + "." + prefix + "." + reference;
                Optional<String> candidateMatch = findKnownType(candidate);
                if (candidateMatch.isPresent()) {
                    return candidateMatch;
                }
            }

            String simple = simpleName(reference);
            String samePackageCandidate = relation.sourcePackage().isBlank()
                    ? simple
                    : relation.sourcePackage() + "." + simple;
            Optional<String> samePackage = findKnownType(samePackageCandidate);
            if (samePackage.isPresent()) {
                return samePackage;
            }

            String imported = relation.explicitImports().get(simple);
            if (imported != null && typesByFqn.containsKey(imported)) {
                return Optional.of(imported);
            }

            for (String importedPackage : relation.starImports()) {
                String wildcardCandidate = importedPackage + "." + simple;
                Optional<String> wildcardMatch = findKnownType(wildcardCandidate);
                if (wildcardMatch.isPresent()) {
                    return wildcardMatch;
                }
            }

            List<TypeNode> nodes = typesBySimpleName.get(simple);
            if (nodes != null && nodes.size() == 1) {
                return Optional.of(nodes.get(0).fqn());
            }

            return Optional.empty();
        }

        private Optional<String> findKnownType(String candidate) {
            if (typesByFqn.containsKey(candidate)) {
                return Optional.of(candidate);
            }
            return Optional.empty();
        }

        private static String simpleName(String qualifiedName) {
            int lastDot = qualifiedName.lastIndexOf('.');
            return lastDot < 0 ? qualifiedName : qualifiedName.substring(lastDot + 1);
        }
    }

    private static final class PackageNode {
        private final String segmentName;
        private final Map<String, PackageNode> children = new TreeMap<>();
        private final List<TypeNode> types = new ArrayList<>();

        private PackageNode(String segmentName) {
            this.segmentName = segmentName;
        }
    }

    private record TypeNode(String fqn, String packageName, List<String> enclosingTypes, String simpleName, String displayName,
                            TypeKind kind, boolean abstractType) {
    }

    private record PendingRelation(String sourceFqn, String sourcePackage, List<String> sourceTypePath, RelationKind relationKind,
                                   String rawTarget, Map<String, String> explicitImports, List<String> starImports) {
    }

    private enum RelationKind {
        EXTENDS,
        IMPLEMENTS
    }

    private enum TypeKind {
        CLASS,
        INTERFACE,
        ENUM,
        RECORD,
        ANNOTATION
    }
}