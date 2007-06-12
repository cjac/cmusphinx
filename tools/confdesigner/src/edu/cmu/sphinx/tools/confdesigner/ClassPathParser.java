package edu.cmu.sphinx.tools.confdesigner;

import edu.cmu.sphinx.util.props.Configurable;
import edu.cmu.sphinx.util.props.ConfigurationManagerUtils;

import java.io.File;
import java.io.IOException;
import java.lang.reflect.Modifier;
import java.net.JarURLConnection;
import java.net.URL;
import java.net.URLClassLoader;
import java.util.*;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;
import java.util.jar.JarInputStream;
import java.util.jar.Manifest;

/**
 * Some static utility methods which ease to find all configurables within the class-path of the virtual machine.
 *
 * @author Holger Brandl
 */
public class ClassPathParser {

    /** Extracts all <code>Configurable</code>s from the current classpath. */
    public static List<Class<? extends Configurable>> getAllConfigsFromClassPath() throws IOException {
        List<Class<? extends Configurable>> configs = new ArrayList<Class<? extends Configurable>>();

        ClassLoader loader = ClassPathParser.class.getClassLoader();
        System.out.println("the loader is " + loader);


        if (System.getProperty("jnlpx.jvm") != null) {
            URL[] urls = ((URLClassLoader) loader).getURLs();

            for (URL url : urls) {
                String urlName = url.toString();
                if (urlName.contains("jsapi.jar") ||
                        urlName.contains("org-netbeans-modules-visual-examples.jar") ||
                        urlName.contains("l2fprod-common-sheet.jar") ||
                        urlName.contains("org-openide-util.jar") ||
                        urlName.contains("org-netbeans-api-visual.jar") ||
                        urlName.contains("xstream-1.2.jar"))
                    continue;

                System.out.println("url is " + url);
                URL jarURL = new URL("jar:" + url.toString() + "!/");

                JarURLConnection conn = (JarURLConnection) jarURL.openConnection();
                configs.addAll(extractConfigsFromJar(conn.getJarFile()));
            }

        } else {
            System.out.println("the class path is " + System.getProperty("java.class.path"));
            List<String> classPathEntries = new ArrayList<String>(Arrays.asList(System.getProperty("java.class.path").split(File.pathSeparator)));

            // add all classpath from nested jars within the confdesigner manifest if ConDesigner is running from a jar
            java.net.URL codeBase = ClassPathParser.class.getProtectionDomain().getCodeSource().getLocation();
            if (codeBase.getPath().endsWith(".jar")) {
                System.out.println("*** running from jar!");

                JarInputStream jin = new JarInputStream(codeBase.openStream());

                Manifest mf = jin.getManifest();
                classPathEntries.addAll(Arrays.asList(mf.getMainAttributes().getValue("Class-Path").split(" ")));
            }

            // try to find all configurables within the classpath
            for (String classPathEntry : classPathEntries) {
                if (classPathEntry.contains("Java" + File.separator + "jdk") ||
                        classPathEntry.contains("Java" + File.separator + "jre") ||
                        classPathEntry.contains("jsapi.jar") ||
                        classPathEntry.contains("JetBrains" + File.separator + "IntelliJ") ||
                        classPathEntry.contains("org-netbeans-modules-visual-examples.jar") ||
                        classPathEntry.contains("l2fprod-common-sheet.jar") ||
                        classPathEntry.contains("org-openide-util.jar") ||
                        classPathEntry.contains("org-netbeans-api-visual.jar") ||
                        classPathEntry.contains("xstream-1.2.jar"))
                    continue;

                System.err.println("parsing '" + classPathEntry + "' ...");
                if (classPathEntry.endsWith(".jar")) {
                    configs.addAll(extractConfigsFromJar(new JarFile(classPathEntry)));
                } else
                    configs.addAll(extractConfigsFromFileSystem(classPathEntry));
            }
        }

        return configs;
    }


    private static boolean isRunningFromJar() {
        String className = ClassPathParser.class.getName().replace('.', '/');
        String classJar = ClassPathParser.class.getClass().getResource("/" + className + ".class").toString();

        return classJar.startsWith("jar:");
    }


    private static Collection<? extends Class<? extends Configurable>> extractConfigsFromFileSystem(String classPathEntry) {
        File pathEntryFile = new File(classPathEntry);
        List<File> files = findClassFiles(new File(classPathEntry));
        List<Class<? extends Configurable>> configs = new ArrayList<Class<? extends Configurable>>();

        for (File file : files) {
            String fileName = file.getPath();


            if (fileName.endsWith(".class")) {
                try {
                    String className = fileName.replace(pathEntryFile.getPath() + File.separator, "");
                    className = className.replace(File.separator, ".").replace(".class", "").replaceAll("^[.]", "");

                    //remove anything but the package name
                    Class aClass = Class.forName(className);


                    if (ConfigurationManagerUtils.isImplementingInterface(aClass, Configurable.class) && !Modifier.isAbstract(aClass.getModifiers()))
                        configs.add((Class<? extends Configurable>) aClass);
                } catch (ClassNotFoundException e1) {
                    e1.printStackTrace();
                }
            }
        }


        return configs;
    }


    private static Collection<Class<? extends Configurable>> extractConfigsFromJar(JarFile jarFile) throws IOException {
        List<Class<? extends Configurable>> configs = new ArrayList<Class<? extends Configurable>>();

        Enumeration e = jarFile.entries();
        while (e.hasMoreElements()) {
            JarEntry o = (JarEntry) e.nextElement();
            String entryName = o.getName();

            if (entryName.endsWith(".class")) {
                try {
                    entryName = entryName.replace("/", ".").replace(".class", "");
                    System.err.println(entryName);
                    Class aClass = Class.forName(entryName);
                    if (ConfigurationManagerUtils.isImplementingInterface(aClass, Configurable.class) && !Modifier.isAbstract(aClass.getModifiers()))
                        configs.add((Class<? extends Configurable>) aClass);
                } catch (ClassNotFoundException e1) {
                    e1.printStackTrace();
                } catch (Throwable t) {
                    t.printStackTrace();
                }
            }
        }

        return configs;
    }


    /** Recursively parses a directory and put all files in the return list which match ALL given filters. */
    public static List<File> findClassFiles(File directory) {
        assert directory.isDirectory();

        List<File> allFiles = new ArrayList<File>();

        File[] files = directory.listFiles();
        for (File file : files) {
            if (file.isDirectory())
                allFiles.addAll(findClassFiles(file));
            else if (file.isFile() && file.getName().endsWith(".class")) {
                allFiles.add(file);
            }
        }

        return allFiles;
    }
}
