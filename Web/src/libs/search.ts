
export interface MatchResult {
  value: string
  dist: number
}

function lcsLength(a: string, b: string): number {
  if (a.length < b.length) {
    [a, b] = [b, a]
  }

  const prev = new Uint16Array(b.length + 1)
  const curr = new Uint16Array(b.length + 1)

  for (let i = 1; i <= a.length; i++) {
    for (let j = 1; j <= b.length; j++) {
      if (a[i - 1] === b[j - 1]) {
        curr[j] = prev[j - 1]! + 1
      } else {
        curr[j] = Math.max(prev[j]!, curr[j - 1]!)
      }
    }
    prev.set(curr)
  }

  return prev[b.length]!
}


export function stringSearch(
  list: string[],
  query: string,
  limit = 5
): MatchResult[] {
  const q = query.toLowerCase()

  let results: MatchResult[] = []

  for (const value of list) {
    const lcs = lcsLength(q, value.toLowerCase())
    results.push({ value, dist: lcs })
  }

  // Higher LCS = better match
  results.sort((a, b) => b.dist - a.dist)

  return results.slice(0, limit)
}
