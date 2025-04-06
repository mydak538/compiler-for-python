print("✅ Компилятор работает!")
print(f"Версия: {'.'.join(map(str, [3, 14]))}")

def test(n):
    return sum(x*x for x in range(n))

if __name__ == "__main__":
    result = test(1_000_000)
    print(f"Результат вычислений: {result:,}")
