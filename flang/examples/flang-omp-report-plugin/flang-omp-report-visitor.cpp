//===-- examples/flang-omp-report-plugin/flang-omp-report-visitor.cpp -----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "flang-omp-report-visitor.h"

namespace Fortran {
namespace parser {
bool operator<(const ClauseInfo &a, const ClauseInfo &b) {
  return a.clause < b.clause;
}
bool operator==(const ClauseInfo &a, const ClauseInfo &b) {
  return a.clause == b.clause && a.clauseDetails == b.clauseDetails;
}
bool operator!=(const ClauseInfo &a, const ClauseInfo &b) { return !(a == b); }

bool operator==(const LogRecord &a, const LogRecord &b) {
  return a.file == b.file && a.line == b.line && a.construct == b.construct &&
      a.clauses == b.clauses;
}
bool operator!=(const LogRecord &a, const LogRecord &b) { return !(a == b); }

std::string OpenMPCounterVisitor::normalize_construct_name(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(),
      [](unsigned char c) { return std::tolower(c); });
  return s;
}
ClauseInfo OpenMPCounterVisitor::normalize_clause_name(const std::string &s) {
  std::size_t start = s.find('(');
  std::size_t end = s.find(')');
  std::string clauseName;
  if (start != std::string::npos && end != std::string::npos) {
    clauseName = s.substr(0, start);
    clauseDetails = s.substr(start + 1, end - start - 1);
  } else {
    clauseName = s;
  }
  std::transform(clauseName.begin(), clauseName.end(), clauseName.begin(),
      [](unsigned char c) { return std::tolower(c); });
  std::transform(clauseDetails.begin(), clauseDetails.end(),
      clauseDetails.begin(), [](unsigned char c) { return std::tolower(c); });
  return ClauseInfo{clauseName, clauseDetails};
}
SourcePosition OpenMPCounterVisitor::getLocation(const OmpWrapperType &w) {
  if (auto *val = std::get_if<const OpenMPConstruct *>(&w)) {
    const OpenMPConstruct *o{*val};
    return getLocation(*o);
  }
  return getLocation(*std::get<const OpenMPDeclarativeConstruct *>(w));
}
SourcePosition OpenMPCounterVisitor::getLocation(
    const OpenMPDeclarativeConstruct &c) {
  return std::visit(
      [&](const auto &o) -> SourcePosition {
        return parsing->allCooked().GetSourcePositionRange(o.source)->first;
      },
      c.u);
}
SourcePosition OpenMPCounterVisitor::getLocation(const OpenMPConstruct &c) {
  return std::visit(
      Fortran::common::visitors{
          [&](const OpenMPStandaloneConstruct &c) -> SourcePosition {
            return parsing->allCooked().GetSourcePositionRange(c.source)->first;
          },
          // OpenMPSectionsConstruct, OpenMPLoopConstruct,
          // OpenMPBlockConstruct, OpenMPCriticalConstruct Get the source from
          // the directive field.
          [&](const auto &c) -> SourcePosition {
            const CharBlock &source{std::get<0>(c.t).source};
            return (parsing->allCooked().GetSourcePositionRange(source))->first;
          },
          [&](const OpenMPAtomicConstruct &c) -> SourcePosition {
            return std::visit(
                [&](const auto &o) -> SourcePosition {
                  const CharBlock &source{std::get<Verbatim>(o.t).source};
                  return parsing->allCooked()
                      .GetSourcePositionRange(source)
                      ->first;
                },
                c.u);
          },
      },
      c.u);
}

std::string OpenMPCounterVisitor::getName(const OmpWrapperType &w) {
  if (auto *val = std::get_if<const OpenMPConstruct *>(&w)) {
    const OpenMPConstruct *o{*val};
    return getName(*o);
  }
  return getName(*std::get<const OpenMPDeclarativeConstruct *>(w));
}
std::string OpenMPCounterVisitor::getName(const OpenMPDeclarativeConstruct &c) {
  return std::visit(
      [&](const auto &o) -> std::string {
        const CharBlock &source{std::get<Verbatim>(o.t).source};
        return normalize_construct_name(source.ToString());
      },
      c.u);
}
std::string OpenMPCounterVisitor::getName(const OpenMPConstruct &c) {
  return std::visit(
      Fortran::common::visitors{
          [&](const OpenMPStandaloneConstruct &c) -> std::string {
            return std::visit(
                [&](const auto &c) {
                  // Get source from the directive or verbatim fields
                  const CharBlock &source{std::get<0>(c.t).source};
                  return normalize_construct_name(source.ToString());
                },
                c.u);
          },
          [&](const OpenMPExecutableAllocate &c) -> std::string {
            const CharBlock &source{std::get<0>(c.t).source};
            return normalize_construct_name(source.ToString());
          },
          [&](const OpenMPDeclarativeAllocate &c) -> std::string {
            const CharBlock &source{std::get<0>(c.t).source};
            return normalize_construct_name(source.ToString());
          },
          [&](const OpenMPAtomicConstruct &c) -> std::string {
            return std::visit(
                [&](const auto &c) {
                  // Get source from the verbatim fields
                  const CharBlock &source{std::get<Verbatim>(c.t).source};
                  return "atomic-" +
                      normalize_construct_name(source.ToString());
                },
                c.u);
          },
          // OpenMPSectionsConstruct, OpenMPLoopConstruct,
          // OpenMPBlockConstruct, OpenMPCriticalConstruct Get the source from
          // the directive field of the begin directive or from the verbatim
          // field of the begin directive in Critical
          [&](const auto &c) -> std::string {
            const CharBlock &source{std::get<0>(std::get<0>(c.t).t).source};
            return normalize_construct_name(source.ToString());
          },
      },
      c.u);
}

bool OpenMPCounterVisitor::Pre(const OpenMPDeclarativeConstruct &c) {
  OmpWrapperType *ow{new OmpWrapperType(&c)};
  ompWrapperStack.push_back(ow);
  return true;
}
bool OpenMPCounterVisitor::Pre(const OpenMPConstruct &c) {
  OmpWrapperType *ow{new OmpWrapperType(&c)};
  ompWrapperStack.push_back(ow);
  return true;
}
bool OpenMPCounterVisitor::Pre(const OmpEndLoopDirective &c) { return true; }
bool OpenMPCounterVisitor::Pre(const DoConstruct &) {
  loopLogRecordStack.push_back(curLoopLogRecord);
  return true;
}

void OpenMPCounterVisitor::Post(const OpenMPDeclarativeConstruct &) {
  PostConstructsCommon();
}
void OpenMPCounterVisitor::Post(const OpenMPConstruct &) {
  PostConstructsCommon();
}
void OpenMPCounterVisitor::PostConstructsCommon() {
  OmpWrapperType *curConstruct = ompWrapperStack.back();
  std::sort(
      clauseStrings[curConstruct].begin(), clauseStrings[curConstruct].end());

  SourcePosition s{getLocation(*curConstruct)};
  LogRecord r{s.file.path(), s.line, getName(*curConstruct),
      clauseStrings[curConstruct]};
  constructClauses.push_back(r);

  // Keep track of loop log records if it can potentially have the
  // nowait clause added on later.
  if (const auto *oc = std::get_if<const OpenMPConstruct *>(curConstruct)) {
    if (const auto *olc = std::get_if<OpenMPLoopConstruct>(&(*oc)->u)) {
      const auto &beginLoopDir{
          std::get<Fortran::parser::OmpBeginLoopDirective>(olc->t)};
      const auto &beginDir{
          std::get<Fortran::parser::OmpLoopDirective>(beginLoopDir.t)};
      if (beginDir.v == llvm::omp::Directive::OMPD_do ||
          beginDir.v == llvm::omp::Directive::OMPD_do_simd) {
        curLoopLogRecord = &constructClauses.back();
      }
    }
  }

  auto it = clauseStrings.find(curConstruct);
  clauseStrings.erase(it);
  ompWrapperStack.pop_back();
  delete curConstruct;
}
void OpenMPCounterVisitor::Post(const OmpEndLoopDirective &c) {}

void OpenMPCounterVisitor::Post(const OmpProcBindClause::Type &c) {
  clauseDetails += "type=" + OmpProcBindClause::EnumToString(c) + ";";
}
void OpenMPCounterVisitor::Post(const OmpDefaultClause::Type &c) {
  clauseDetails += "type=" + OmpDefaultClause::EnumToString(c) + ";";
}
void OpenMPCounterVisitor::Post(
    const OmpDefaultmapClause::ImplicitBehavior &c) {
  clauseDetails +=
      "implicit_behavior=" + OmpDefaultmapClause::EnumToString(c) + ";";
}
void OpenMPCounterVisitor::Post(
    const OmpDefaultmapClause::VariableCategory &c) {
  clauseDetails +=
      "variable_category=" + OmpDefaultmapClause::EnumToString(c) + ";";
}
void OpenMPCounterVisitor::Post(const OmpScheduleModifierType::ModType &c) {
  clauseDetails += "modifier=" + OmpScheduleModifierType::EnumToString(c) + ";";
}
void OpenMPCounterVisitor::Post(const OmpLinearModifier::Type &c) {
  clauseDetails += "modifier=" + OmpLinearModifier::EnumToString(c) + ";";
}
void OpenMPCounterVisitor::Post(const OmpDependenceType::Type &c) {
  clauseDetails += "type=" + OmpDependenceType::EnumToString(c) + ";";
}
void OpenMPCounterVisitor::Post(const OmpMapType::Type &c) {
  clauseDetails += "type=" + OmpMapType::EnumToString(c) + ";";
}
void OpenMPCounterVisitor::Post(const OmpScheduleClause::ScheduleType &c) {
  clauseDetails += "type=" + OmpScheduleClause::EnumToString(c) + ";";
}
void OpenMPCounterVisitor::Post(const OmpIfClause::DirectiveNameModifier &c) {
  clauseDetails += "name_modifier=" + OmpIfClause::EnumToString(c) + ";";
}
void OpenMPCounterVisitor::Post(const OmpCancelType::Type &c) {
  clauseDetails += "type=" + OmpCancelType::EnumToString(c) + ";";
}
void OpenMPCounterVisitor::Post(const OmpClause &c) {
  PostClauseCommon(normalize_clause_name(c.source.ToString()));
  clauseDetails.clear();
}
void OpenMPCounterVisitor::PostClauseCommon(const ClauseInfo &ci) {
  // The end loop construct (!$omp end do) can contain a nowait clause.
  // The flang parser does not parse the end loop construct as part of
  // the OpenMP construct for the loop construct. So the end loop is left
  // hanging as a separate executable construct. If a nowait clause is seen in
  // an end loop construct we have to find the associated loop construct and
  // add nowait to its list of clauses. Note: This is not a bug in flang, the
  // parse tree is corrected during semantic analysis.
  if (ci.clause == "nowait") {
    assert(curLoopLogRecord &&
        "loop Construct should be visited before a nowait clause");
    constructClauseCount[std::make_pair(
        curLoopLogRecord->construct, ci.clause)]++;
    curLoopLogRecord->clauses.push_back(ci);
  } else {
    assert(!ompWrapperStack.empty() &&
        "Construct should be visited before clause");
    constructClauseCount[std::make_pair(
        getName(*ompWrapperStack.back()), ci.clause)]++;
    clauseStrings[ompWrapperStack.back()].push_back(ci);
  }
}
void OpenMPCounterVisitor::Post(const DoConstruct &) {
  curLoopLogRecord = loopLogRecordStack.back();
  loopLogRecordStack.pop_back();
}
} // namespace parser
} // namespace Fortran
