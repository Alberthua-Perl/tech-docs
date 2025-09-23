## ☕ Java 基础与进阶

### Java 运行时环境

- HostSpot JVM：
  
  HotSpot JVM 是 Java 平台上广泛使用的 Java 虚拟机（JVM）实现，由 Oracle Corporation（原 Sun Microsystems）开发。它是 Java Development Kit（JDK）的一部分，负责执行 Java 字节码，并提供了运行 Java 应用程序所需的运行时环境。
  
  HotSpot JVM 的主要特点包括：
  
  1. **即时编译（JIT）**：HotSpot JVM 使用即时编译器（Just-In-Time Compiler）来提高程序的执行效率。它在程序运行时动态地将 Java 字节码编译成机器码，这样可以在不牺牲启动速度的情况下提高程序的运行性能。
  
  2. **垃圾回收（GC）**：HotSpot JVM 提供了多种垃圾回收器，用于自动管理内存，回收不再被引用的对象，防止内存泄漏。这些垃圾回收器包括 Serial GC、Parallel GC、Concurrent Mark Sweep（CMS）GC 和 G1 GC 等，它们适用于不同的应用场景和性能需求。
  
  3. **多版本并发控制（MCS）**：HotSpot JVM 使用多版本并发控制来优化内存分配和垃圾回收，减少应用程序的暂停时间，提高响应速度。
  
  4. **自适应优化**：HotSpot JVM 的编译器能够根据程序的运行情况自适应地调整优化策略，例如通过内联（Inlining）来减少方法调用的开销，或者通过逃逸分析（Escape Analysis）来优化对象的存储。
  
  5. **线程管理**：HotSpot JVM 提供了高效的线程管理机制，支持多处理器和多核系统，能够充分利用现代硬件的并行处理能力。
  
  6. **性能监控和诊断工具**：HotSpot JVM 提供了一系列工具，如 jconsole、jvisualvm 和 jstat，帮助开发者监控和诊断应用程序的性能问题。
  
  HotSpot JVM 在 Java 社区中非常流行，因为它提供了高性能和良好的兼容性，是许多 Java 应用程序的首选运行时环境。随着 Java 版本的更新，HotSpot JVM 也在不断地进行优化和改进，以适应新的硬件和软件需求。

- GraalVM：
  
  GraalVM 是一个由 Oracle Labs 开发的高性能虚拟机，它与 JVM（Java Virtual Machine，Java 虚拟机）有着密切的关系，但同时也提供了超越传统 JVM 的功能。以下是 GraalVM 与 JVM 的主要关系点：
  
  1. **兼容 JVM**：GraalVM 是基于 OpenJDK（Open Java Development Kit）构建的，这意味着它完全兼容 Java 应用程序。你可以将运行在 HotSpot JVM 上的 Java 代码无缝迁移到 GraalVM 上，而不需要做任何修改。
  
  2. **多语言支持**：GraalVM 的一个显著特点是它支持多种编程语言，包括 Java、JavaScript、Python、Ruby、R、C 和 C++ 等。这是通过 GraalVM 的 Truffle 框架实现的，它允许这些语言在同一个运行时环境中运行，并且可以相互调用。
  
  3. **即时编译器（JIT）**：GraalVM 提供了自己的即时编译器，称为 Graal 编译器，它可以替代 HotSpot JVM 中的 C2 编译器。Graal 编译器使用 Java 实现，并且提供了更激进的优化策略，这在某些情况下可以提供更好的性能。
  
  4. **原生镜像（Native Image）**：GraalVM 的一个核心特性是能够将 Java 应用程序编译成原生可执行文件，这些文件可以在没有 JVM 的情况下运行。这种 Ahead-of-Time（AOT）编译技术可以显著减少应用程序的启动时间，并降低内存占用，这对于云原生应用和微服务架构特别有用。
  
  5. **多语言互操作性**：GraalVM 支持在不同语言之间进行互操作，这意味着你可以在 Java 应用程序中直接调用 JavaScript、Python 或其他语言的代码，而不需要通过网络或其他中间层。
  
  6. **社区和企业版**：GraalVM 提供了社区版和企业版。社区版基于 OpenJDK，而企业版则基于 Oracle JDK，提供了额外的性能分析和优化工具。
  
  总的来说，GraalVM 是一个多语言、高性能的虚拟机，它扩展了传统 JVM 的功能，提供了更好的性能和更灵活的部署选项。虽然它与 JVM 紧密相关，但它的目标是提供一个更现代、更高效的运行时环境，以满足现代应用程序的需求。

### 常用框架

- Swagger

- Quarkus：云原生框架，已集成 Swagger。

### 常用模块的 Java API

- io.quarkus.test.junit.QuarkusTest：
  
  注解：@QuarkusTest

- org.junit.jupiter.api.BeforeEach：
  
  注解：@BeforeEach

- org.junit.jupiter.api.Test：
  
  注解：@Test

- org.junit.jupiter.api.Assertions.assertEquals

- org.junit.jupiter.api.Assertions.assertThrows

- javax.inject.Inject：
  
  - 注解：@Inject

### private 访问修饰符

在 Java 中，`private` 是一个访问修饰符，用于限制类成员（字段、方法）的访问级别。当在一个类中声明一个成员（如变量或方法）为 `private` 时，限制了这个成员只能在定义它的类内部被访问。这意味着其他类（包括子类）无法直接访问这些 `private` 成员。

  以下是 `private` 关键字的一些关键特性：

- **访问限制**：`private` 成员只能在其所属的类内部被访问。如果尝试从类的外部访问这些成员，编译器会报错。

- **封装**：`private` 关键字是实现封装（Encapsulation）原则的基础，封装是面向对象编程（OOP）的核心概念之一。封装允许隐藏类的内部实现细节，只暴露出必要的接口。

- **安全性**：通过限制对类内部数据的直接访问，`private` 关键字有助于保护数据的完整性，防止外部代码的不当操作。

- **灵活性**：虽然 `private` 成员不能被外部类直接访问，但可以通过公共的 getter 和 setter 方法（或者在 Java Bean 模式中）来提供对这些成员的安全访问。这样，可以在这些方法中添加逻辑来控制数据的读写，例如验证数据的有效性。
  
  下面是一个使用 `private` 关键字的例子：
  
  ```java
  public class Example {
    private int privateValue; // 私有成员，只能在Example类内部访问
  
    public int getPrivateValue() { // 提供公共的getter方法
        return privateValue;
    }
  
    public void setPrivateValue(int value) { // 提供公共的setter方法
        this.privateValue = value;
    }
  }
  ```
  
  在这个例子中，privateValue 是一个私有成员，它不能被外部类直接访问。但是，通过 getPrivateValue 和 setPrivateValue 方法，外部类可以安全地读取和修改这个私有成员的值。

### this 关键字的多种用途

`this` 关键字是 Java 面向对象编程中的一个重要特性，它提供了一种清晰、灵活的方式来引用当前对象的状态和行为。在 Java 中，`this` 关键字有多种用途，主要用于以下几个方面：

- **区分成员变量和局部变量**：当方法中的局部变量与类的成员变量同名时，使用 `this` 关键字可以明确指出你想要引用的是成员变量。
  
  ```java
  public class Example {
      private int value;
  
      public void setValue(int value) {
          this.value = value; // 使用 this 来引用成员变量
      }
  }
  ```

- **链式调用**：在构造函数或方法中，`this` 可以用来返回当前对象的引用，从而实现链式调用。
  
  ```java
  public class Example {
      private int value;
  
      public Example setValue(int value) {
          this.value = value;
          return this; // 返回当前对象的引用，实现链式调用。
      }
  }
  ```

- **调用当前类的其他构造函数**：在构造函数中，`this` 可以用来调用同一个类中的另一个构造函数。
  
  ```java
  public class Example {
      private int value;
  
      public Example() {
          this(0); // 调用带参数的构造函数
      }
  
      public Example(int value) {
          this.value = value;
      }
  }
  ```

- **在内部类中引用外部类的成员**：在内部类中，`this` 用来引用外部类的当前对象。
  
  ```java
  public class Outer {
      private int outerValue;
  
      public class Inner {
          public void printValues() {
              System.out.println("Outer value: " + this.outerValue);
              // 使用 this 来引用外部类的成员
          }
      }
  }
  ```

- **在匿名内部类中引用外部类的成员**：与内部类类似，`this` 也可以用来引用匿名内部类外部的成员。
  
  ```java
  public class Example {
      private int value;
  
      public Example createInner() {
          return new Example() {
              public void printValue() {
                  System.out.println("Value: " + this.value);
                  // 使用 this 来引用外部类的成员
              }
          };
      }
  }
  ```
