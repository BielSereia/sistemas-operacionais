const data = Array.from({ length: 1000000 }, () => "x".repeat(100));
console.log("Criado:", data.length, "itens", process.pid);
process.stdin.resume();