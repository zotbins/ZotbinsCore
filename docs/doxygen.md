# Intro to Doxygen

Doxygen is documentation generator tool from annotated C++ sources.

It generates directly from the C++ source code, which makes it easier to keep the documentation consistant from the source code.

## Documentating Source Code
In order to have the documentation generate correctly, you need to add a file header with an description about the file. This needs to be present in every file that needs documentation.

```
/**
 * @file foo.hpp
 * @brief Header file for Foo class
 * @version 0.1
 * @date 2022-04-27
 *
 */
```

To document functions, you need to add Doxygen-style comments
```
/**
 * @brief Calculates the area of circle
 * @param[in] radius of circle
 * @return float Area
 */
float calc_area(float radius)
{
    ...
}

/**
 * @brief Calculates quotient
 * @param[in] top numerator
 * @param[in] bot denominator
 * @param[out] remainder
 * @return int quotient
 */
int divide(int top, int bot, int &remainder)
{
    ...
}

```

## Example Source File
```
/**
 * @file foo.hpp
 * @brief Header file for Foo class
 * @version 0.1
 * @date 2022-04-27
 *
 */

/**
 * @brief Example class
 *
 */
class Foo
{
public:
    /**
     * @brief Construct a new Foo object.
     * @param x Parameter
     *
     */
    Foo(int x);

    /**
     * @brief Returns bar
     * @return int bar
     *
     */
    int bar();
};

```

## Running Doxygen Locally

Before proceeding, you need to install [Doxygen](https://www.doxygen.nl/download.html). It is supported on Windows, Mac, and Linux.

In order to generate the UML diagrams, you need to have [PlantUML](https://plantuml.com/download) installed. Install the PlantUML compiled Jar and update `PLANTUML_JAR_PATH` to the appropriate value. For example, if you are on Windows and the PlantUML jar is located in your Downloads folder, `PLANTUML_JAR_PATH` probably is `C:\Users\{YOUR NAME}\Downloads\plantuml.jar`

After installing, run the `doxygen` command in the `.doxygen/` directory, and this should generate the HTML documentation files for ZotBins Core.

## Doxygen Github Action

Every commit to `main` will start the Doxygen Github Action, which is loaded in `.github/workflow/doxygen.yml`. This generates HTML files using Doxygen and deploys it to Github Pages.